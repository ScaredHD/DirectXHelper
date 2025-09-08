# Instancing and CPU Frustum Culling

Drawing large amount of boxes. Press 'C' to toggle CPU frustum culling.

## Culling method

Test on 13700K(F).

### Method 1 (Cull in local space)

- Single threaded
- Per object AABB-frustum intersection test
  - Transform NDC frustum planes to object local space with inverse of MVP
    - N matrix inverse operations
  - Test intersection in local space with box bounding volume [-0.5, 0.5]^3

- Time to cull (1M boxes)
  - -O0: 280ms
  - -O3: 50ms

### Method 2 (Cull in world space)

- Single threaded
- Transform box AABB from local space to world space with M
  - N matrix multiplication
- Transform NDC frustum planes to world space with inverse of VP
  - 1 operation
- Test intersection in world space

- Time to cull (1M boxes)
  - -O0: 
  - -O3: 

### Method 3 (Octree)

- Single threaded
- Cull in world space
- Accelerate intersection testing with octree
