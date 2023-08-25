## How to build
### Debug and Tests
```
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -Dlibsercli_BUILD_CTESTS=ON -Dlibsercli_BUILD_UTESTS=ON ..
make
```
