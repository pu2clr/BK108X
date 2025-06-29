# Windows Scripts for BK108X Arduino Library

This folder contains Windows batch files (.bat) equivalent to the Unix shell scripts, designed to help Windows users set up and compile the BK108X Arduino Library examples.

## Files

- **config_libraries_and_boards.bat** - Sets up the Arduino development environment
- **compile_all.bat** - Compiles all example sketches for different board types
- **README_Windows.md** - This file

## Prerequisites

- Windows 10 or later
- Internet connection
- Administrator privileges (recommended for installation)

## Quick Start

### Step 1: Environment Setup

1. Open Command Prompt as Administrator (recommended)
2. Navigate to the examples folder where these .bat files are located
3. Run the configuration script:
   ```cmd
   config_libraries_and_boards.bat
   ```

This script will:
- Download and install arduino-cli
- Configure board manager URLs for various Arduino-compatible boards
- Install all required libraries
- Install board packages (Arduino AVR, ESP32, STM32, etc.)

**Note:** This process may take 30 minutes to several hours depending on your internet connection speed.

### Step 2: Compile Examples

After the environment is set up, you can compile all examples by running:
```cmd
compile_all.bat
```

This will compile sketches for:
- Arduino ATmega328 based boards (Nano, Uno, etc.)
- LGT8FX based boards
- ESP32 boards

Compiled files will be saved to `%USERPROFILE%\Downloads\hex\` with organized subfolders.

## Manual PATH Configuration

If you want to use arduino-cli from any location, add `%USERPROFILE%\bin` to your Windows PATH:

1. Press `Win + R`, type `sysdm.cpl`, press Enter
2. Click "Environment Variables"
3. Under "User variables", select "Path" and click "Edit"
4. Click "New" and add: `%USERPROFILE%\bin`
5. Click "OK" to save

## Troubleshooting

### Arduino-CLI Not Found
If you get "arduino-cli is not recognized as an internal or external command":
- Make sure the installation completed successfully
- Check if `%USERPROFILE%\bin\arduino-cli.exe` exists
- Add `%USERPROFILE%\bin` to your PATH as described above

### Network Issues
If downloads fail:
- Check your internet connection
- Try running the script as Administrator
- Temporarily disable antivirus software during installation

### Compilation Errors
If compilation fails:
- Ensure all libraries were installed successfully
- Check that the required board packages are installed
- Verify the sketch paths in `compile_all.bat` match your folder structure

### PowerShell Execution Policy
If PowerShell commands fail, you may need to allow script execution:
```cmd
powershell -Command "Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser"
```

## Advanced Usage

### Customizing Compilation

You can modify `compile_all.bat` to:
- Add or remove board types
- Change output directories
- Add or remove warning levels
- Include additional sketches

### Adding More Boards

To add support for additional boards, edit `config_libraries_and_boards.bat` and add the board package URLs to the configuration section.

## Support

For issues specific to these Windows scripts, please check:
1. The main BK108X library documentation
2. Arduino CLI documentation: https://arduino.github.io/arduino-cli/
3. The original Unix scripts for comparison

## License

These scripts are provided under the same license as the BK108X Arduino Library (MIT License).
