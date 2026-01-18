## Compile Project

### Windows
`cmake -G Ninja -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_BUILD_TYPE=Release -S . -B Build`

### Linux
`cmake -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_EXE_LINKER_FLAGS=-fuse-ld=lld -DCMAKE_BUILD_TYPE=Release -S . -B Build`

### Build
`cmake --build Build --config Release`

## Compile Nvidia PhysX

`cmake . -DPX_GENERATE_STATIC_LIBRARIES=ON -DCMAKE_BUILD_TYPE=Release`\
`cmake --build . --config Release`
