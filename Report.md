# 11/23

# OpenGL 포팅 이슈 리포트: 렌더링 파이프라인 및 텍스트 시스템 수정

## 1. Texture::Draw 수정: Pivot Point 및 행렬 순서

### 1.1 문제 상황 (Problem)

Raylib의 Immediate Mode 렌더링을 OpenGL 커스텀 엔진으로 이식하는 과정에서, `Texture::Draw` 함수의 변환 행렬(Transformation Matrix) 적용 방식에 차이가 발생했습니다.

- **Raylib:** 텍스처의 **좌상단(Top-Left) 모서리**를 기준점(Pivot)으로 회전 및 이동합니다.

- **초기 구현:** 객체의 **중심(Center)**을 기준으로 회전한 뒤 이동하는 방식(`T * M * S`)으로 구현되어, 회전 축이 어긋나는 현상이 발생했습니다.

### 1.2 해결 방안 (Solution)

Raylib과 동일하게 모서리를 기준으로 변환하기 위해 행렬 곱셈 순서를 변경했습니다.

- **기존 순서 (`T * M * S`):** `Translate(Final) * DisplayMatrix * Scale`
  
  - 객체 중심을 기준으로 회전이 적용됨.

- **수정된 순서 (`M * T * S`):** `DisplayMatrix * Translate(PivotOffset) * Scale`
  
  - `DisplayMatrix`(월드 변환)를 적용하기 **전**에, 쿼드의 중심을 이동시켜 (0,0)이 모서리에 오도록 만듭니다.

**수정된 코드 (`source/Engine/Texture.cpp`):**

```
void Texture::Draw(const Math::TransformationMatrix& display_matrix, Math::ivec2 texel_position, Math::ivec2 frame_size, unsigned int color, float depth)
{
    CS200::IRenderer2D* renderer = Engine::GetTextureManager().GetRenderer2D();

    // ... (UV 좌표 계산 로직 생략) ...

    // 쿼드의 중심(-0.5 ~ 0.5)을 (0 ~ 1)로 옮기기 위한 오프셋
    Math::vec2 half_size{ frame_size.x * 0.5, frame_size.y * 0.5 };

    // [중요 수정] TranslationMatrix 위치 변경
    // 순서: Scale(크기설정) -> Translate(중심점 이동) -> DisplayMatrix(월드 배치)
    // 결과: (0,0) 모서리를 기준으로 DisplayMatrix의 회전/이동이 적용됨
    const auto world_transformation = display_matrix * Math::TranslationMatrix(half_size) * Math::ScaleMatrix(frame_size);

    renderer->DrawQuad(world_transformation, textureHandle, texel_coord_bl, texel_coord_tr, color, depth);
}
```

## 2. 파생된 문제: TextManager 렌더링 오류

### 2.1 증상 (Symptom)

`Texture::Draw`를 수정한 직후, 일반적인 스프라이트는 정상적으로 출력되었으나 `TextManager`를 통해 렌더링되는 텍스트들이 화면에서 사라지는 현상이 발생했습니다.

### 2.2 원인 분석 (Root Cause)

`Font::DrawChar` 함수에는 이미지 좌표계(Top-Left Origin)와 OpenGL 좌표계(Bottom-Left Origin)의 차이를 보정하기 위한 **Y축 반전 행렬(`Flip`)**이 존재했습니다.

```
// Font::DrawChar 내부의 레거시 로직
const auto flip = Math::ScaleMatrix(Math::vec2{ 1, -1 }); // Y축을 -1로 뒤집음
```

이 `flip` 행렬이 변경된 `Texture::Draw`의 행렬 순서(`M * T * S`)와 결합되면서 문제가 발생했습니다.

1. **기존 (`T * M * S`):** `Translate(Pivot)`이 **마지막**에 적용되어, `flip`으로 뒤집힌 좌표를 다시 양수 영역으로 끌어올렸습니다.

2. **변경 (`M * T * S`):** `DisplayMatrix` 자리에 들어온 `flip`이 **가장 먼저** 적용되면서, 이후 적용되는 `Translate` 벡터까지 모두 Y축 반전시켜 버렸습니다.

3. **결과:** 글자의 Y좌표가 음수 영역(`0 ~ -Height`)으로 반전되어 뷰포트 밖으로 사라짐 (Clipping).

## 3. 최종 해결: Font 클래스 위치 보정 (Patch)

### 3.1 해결 전략 (Offset Patch)

`Font` 클래스의 기존 로직을 최대한 유지하면서 문제를 해결하기 위해, `Flip`이 적용되기 직전에 강제로 위치를 보정하는 행렬을 추가했습니다.

- **원리:** `Flip`(`Scale(1, -1)`)을 하기 전에 글자의 높이(`Height`)만큼 Y축 양의 방향으로 미리 이동(`Offset`)시켜 놓습니다.

- **결과:** 뒤집혔을 때 음수 영역으로 떨어지는 대신, 정상적인 화면 영역(`0 ~ Height`)에 안착하게 됩니다.

### 3.2 수정된 코드 (`source/Engine/Font.cpp`)

`Font::DrawChar` 함수에 `offset_up` 행렬을 추가하고 곱셈 순서를 조정했습니다.

```
void Font::DrawChar(Math::TransformationMatrix& matrix, char c, CS200::RGBA color)
{
    const Math::irect& display_rect   = GetCharRect(c);
    const Math::ivec2 top_left_texel = { display_rect.Left(), display_rect.Bottom() };

    if (c != ' ')
    {
        // 1. Y축 반전 행렬 (기존)
        const auto flip = Math::ScaleMatrix(Math::vec2{ 1, -1 });

        // 2. [Patch] 높이만큼 위로 올려주는 오프셋 행렬 생성
        // Flip으로 인해 음수 좌표로 내려갈 것을 대비하여 미리 위로 올림
        const auto offset_up = Math::TranslationMatrix(Math::vec2{ 0.0, static_cast<double>(display_rect.Size().y) });

        // 3. 순서: Matrix(World) * Offset(Local Up) * Flip(Local Invert)
        // 새로운 Texture::Draw 로직(M*T*S) 하에서도 정상 위치(양수 Y)에 그려짐
        texture.Draw(matrix * offset_up * flip, top_left_texel, display_rect.Size(), color);
    }

    // 다음 글자 위치로 이동
    matrix *= Math::TranslationMatrix(Math::ivec2{ display_rect.Size().x, 0 });
}
```

### 3.3 결과

- `Texture::Draw`는 Raylib과 동일한 Pivot(Corner) 기준 변환을 수행합니다.

- `TextManager`는 추가된 Offset 행렬 덕분에 올바른 위치에 텍스트를 렌더링합니다.

# 11/25

# Instanced Rendering 모드 SDF 렌더링 수정 리포트

## 1. 문제 상황 (Issue)

- **증상:** `InstancedRenderer2D` 모드에서 Circle, Rectangle, Line 등의 SDF 기반 도형 렌더링 시 외곽선(Stroke)이 잘리거나 제대로 표현되지 않음.

- **비교:** `ImmediateRenderer2D` 및 `BatchRenderer2D` 모드에서는 정상적으로 렌더링됨.

- **원인:** SDF 렌더링은 외곽선 두께(`LineWidth`)만큼 쿼드(Quad)의 크기를 물리적으로 키워야 하는데, 인스턴싱 모드에서는 이 '확장된 변환(Expanded Transform)'이 적용되지 않고 원본 크기로만 그려지고 있었음.

## 2. 원인 분석 (Root Cause Analysis)

### 2.1. 렌더링 모드별 차이점

- **Batch Rendering:** CPU에서 모든 변환을 미리 계산합니다. `CalculateSDFTransform` 함수가 반환한 **확장된 크기의 행렬**을 사용하여 정점 위치(`x`, `y`)와 텍스처 좌표(`s`, `t`)를 CPU가 직접 계산하여 GPU로 넘깁니다. 따라서 셰이더 수정 없이도 정상 작동했습니다.

- **Instanced Rendering:** CPU는 기본 쿼드 정보와 변환 행렬만 넘기고, **GPU(Vertex Shader)가 정점 변환을 수행**합니다. 기존 코드에서는 **확장되지 않은 원본 행렬**을 GPU로 넘겼기 때문에, 셰이더가 외곽선이 포함될 공간을 확보하지 못해 짤림 현상이 발생했습니다.

### 2.2. 매트릭스 매핑 오류

- `Renderer2DUtils::CalculateSDFTransform`이 반환하는 `QuadTransform`은 **Column-Major (열 우선)** 순서의 1차원 배열입니다.

- 이를 `InstancedRenderer2D`의 인스턴스 데이터(`transformrow0`, `transformrow1`)인 **Row-Major (행 우선)** 데이터로 옮길 때 인덱스 매핑이 올바르지 않아 변환이 뒤틀리는 문제가 있었습니다.

## 3. 해결 방안 (Solution)

### 3.1. C++ 코드 수정 (`source/CS200/InstancedRenderer2D.cpp`)

도형을 그릴 때 `CalculateSDFTransform`을 사용하여 외곽선 두께만큼 확장된 변환 행렬을 구하고, 이를 올바른 인덱스로 매핑하여 인스턴스 데이터에 주입했습니다.

- **주요 변경 사항:**
  
  - `DrawCircle`, `DrawRectangle` 함수 내에서 `sdf_transform.QuadTransform` 사용.
  
  - Column-Major 배열을 Row-Major 인스턴스 변수에 매핑:
    
    - `Row0 (X축)`: 인덱스 `0(m00)`, `3(m01)`, `6(m02)` 사용
    
    - `Row1 (Y축)`: 인덱스 `1(m10)`, `4(m11)`, `7(m12)` 사용

C++

```
// 예시: DrawCircle/DrawRectangle 내부
const auto sdf_transform = Renderer2DUtils::CalculateSDFTransform(transform, line_width);

// Column-Major -> Row-Major 매핑
sdf_instance.transformrow0[0] = sdf_transform.QuadTransform[0]; 
sdf_instance.transformrow0[1] = sdf_transform.QuadTransform[3]; 
sdf_instance.transformrow0[2] = sdf_transform.QuadTransform[6]; 

sdf_instance.transformrow1[0] = sdf_transform.QuadTransform[1]; 
sdf_instance.transformrow1[1] = sdf_transform.QuadTransform[4]; 
sdf_instance.transformrow1[2] = sdf_transform.QuadTransform[7]; 

// 셰이더에는 원본 크기(WorldSize) 전달 (SDF 계산 기준)
sdf_instance.worldSize_x = static_cast<float>(sdf_transform.WorldSize[0]);
sdf_instance.worldSize_y = static_cast<float>(sdf_transform.WorldSize[1]);
```

### 3.2. 셰이더 코드 수정 (`Assets/shaders/InstancedRenderer2D/sdf.vert`)

확장된 쿼드 크기에 맞춰 SDF 계산을 위한 테스트 포인트(`vTestPoint`) 좌표를 보정했습니다.

- **주요 변경 사항:**
  
  - 기존: `vTestPoint = aModelPosition * aWorldSize;` (외곽선 고려 안 함)
  
  - 수정: `vTestPoint = aModelPosition * (aWorldSize + vec2(aLineWidth));` (외곽선 두께 포함)

OpenGL Shading Language

```
// sdf.vert
void main()
{
    // ... (위치 계산) ...

    // [수정] 외곽선 두께만큼 좌표 공간 확장
    vec2 quadSize = aWorldSize + vec2(aLineWidth);
    vTestPoint = aModelPosition * quadSize; 

    // ... (나머지 출력) ...
}
```

## 4. 결과 (Result)

- `InstancedRenderer2D`에서도 외곽선(Stroke)이 잘리지 않고 정확한 두께로 렌더링됩니다.

- Immediate, Batch, Instanced 세 가지 모드 모두 동일한 시각적 결과를 보장합니다.
