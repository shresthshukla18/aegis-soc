# Contributing

Contributions are welcome, especially reproducible bug reports, tests, documentation improvements, and defensive detection improvements.

1. Open an issue describing the problem or proposed change.
2. Keep changes focused and document configuration or API changes.
3. Build with CMake and C++20 before submitting a pull request.
4. Test simulation mode and, where available, libpcap capture in an authorized lab.
5. Never include credentials, private packet captures, or personally identifying network data.

Suggested local checks:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./scripts/demo.sh
```
