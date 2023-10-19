# SchnitzelMotor
A crispy cross platform C/C++ engine. Supports Linux, Windows, and macOS.

# Building the Engine

## Windows Instructions:
1. Download and install clang from [here](https://github.com/llvm/llvm-project/releases/download/llvmorg-16.0.6/LLVM-16.0.6-win64.exe)
2. ![image](https://github.com/Cakez77/SchnitzelMotor/assets/45374095/1ad4bdf5-f43c-4774-9f34-5fcdd2ed7f2c)
3. Download and install git from [here](https://git-scm.com/download/win)
4. Add `sh.exe` to the path, you can find it here : `C:\Program Files\Git\bin`

## Mac Instructions:
1. Ensure you have [Homebrew](https://brew.sh/) installed on your machine.
   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   ```
2. Install dependencies using Homebrew:
   ```bash
   brew update
   brew install llvm glfw freetype
   ```
3. Clone the repository:
   ```bash
   git clone https://github.com/Cakez77/SchnitzelMotor.git
   cd SchnitzelMotor
   ```
4. Run the build script:
   ```bash
   ./build.sh
   ```

## GitHub Actions (macOS)
For automated builds on macOS using GitHub Actions, the provided `mac.yml` workflow file can be utilized. This workflow installs necessary dependencies, checks out the code, and runs the build script:
```yaml
name: mac
on: [push, pull_request]

jobs:
  build:
    runs-on: macos-latest
    steps:
    - name: install dependencies
      run: |
        brew update
        brew install glfw3 freetype
    - name: checkout
      uses: actions/checkout@v3
    - name: build
      run: ./build.sh
```
For more information on GitHub Actions, refer to the [official documentation](https://docs.github.com/en/actions).
