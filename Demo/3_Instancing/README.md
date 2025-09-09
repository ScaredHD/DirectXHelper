# Instancing and CPU Frustum Culling

Drawing large amount of boxes.

## Keyboard controls

C: Toggle frustum culling

O: Toggle octree culling (only effective when frustum culling is enabled)

M: Toggle instance movement. For static objects, scene octree is built once and not updated

## Culling methods

Test on 13700K(F).

### Method 1 (Cull in local space)

- Single threaded
- Per object AABB-frustum intersection test
  - Transform NDC frustum planes to object local space with inverse of MVP
    - N matrix inverse operations
  - Test intersection in local space with box bounding volume [-0.5, 0.5]^3

- Time to cull (1M boxes)
  - -O3: 50ms

### Method 2 (Cull in world space)

- Single threaded
- Transform box AABB from local space to world space with M
  - N matrix multiplication
- Transform NDC frustum planes to world space with inverse of VP
  - 1 operation
- Test intersection in world space

- Time to cull (1M boxes)
  - -O3:

### Method 3 (Octree)

- Single threaded
- Cull in world space
- Accelerate intersection testing with octree
