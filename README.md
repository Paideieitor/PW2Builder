# PW2Builder
Pokémon Generation 5 PMC code injection patch builder  

## How to install
- Install [Java](https://www.java.com/es/download/manual.jsp)
- Install [Git](https://git-scm.com/downloads)
- This builder attaches to your CTRMap project, if you don't already have one you will need to set up that first:
  - Install [CTRMap-CE](https://github.com/ds-pokemon-hacking/CTRMap-CE/releases)
  - Install [CTRMapV](https://github.com/ds-pokemon-hacking/CTRMapV/releases)
  - Create your CTRMap project
  - Install [PMC](https://github.com/ds-pokemon-hacking/PMC/releases) on to your project (you can follow this [guide](https://ds-pokemon-hacking.github.io/docs/generation-v/guides/bw_b2w2-code_injection/#setting-up-the-environment))
- Download the latest release of PW2Builder (if you are not using Windows you will have to build your own executable with CMake and put it in the ``Builder`` folder)
- Download [ARM GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
``Note: If you are in windows download "arm-gnu-toolchain-14.3.rel1-mingw-w64-i686-arm-none-eabi.exe"``
- Open ``Builder\buildSettings.txt``
  - Write the path to your CTRMap project after ``SET PROJECT_DIR=`` (Ex. ``SET PROJECT_DIR=MyFolder\MyCTRMapProject\``)
  - Write the path to your CTRMap.jar file after ``SET CTRMAP_DIR=`` (Ex. ``SET CTRMAP_DIR=MyFolder\CTRMap\``)
  - Write the path to your ARM GNU Toolchain bin folder after ``SET ARM_NONE_EABI_DIR=`` (leave like it is if you set that folder in the systems path)
  - Write the path to your Java after ``SET JAVA_DIR=`` (leave like it is if you set that folder in the systems path)

## How to use
To build a patch you first need to create a build project
The project file structure is as follows:
  - ``Build Project Name``
    - ``Assets`` <- files the patches will insert into the ROMs filesystem
      - ``whitelist.txt`` <- defines the assets that will be inserted depending on the setting definitions
    - ``Externals`` <- external libraries like "NitroKernel" or "ExtLib"
    - ``Global`` <- code that will be shared between patches
    - ``Headers`` <- header files to include in any file
    - ``Patches`` <- folders containig the code for each patch
    - ``Libraries`` <- code to be loaded dynamically by the patches
      - ``whitelist.txt`` <- defines the library folders that will be built depending on the setting definitions
    - ``ESDB.yml`` <- list of symbols that pair the game functions with their location in RAM
    - ``settings.h`` <- definitions to allow customization when installing the patches

### Compilable Files
This builder uses PMC so it can compile *.cpp*, *.c* and *.S* files, and turn them into a *.dll* files  
### Whitelist Sintax
Each whitelisted item needs to be in its own line in the file  
You can use pre-compiler conditionals using the definitions in the ``settings.h`` file to enable and disable the libraries compiled depending on the active features  
You can define a group by writting 2 items in the same line separated by ``<...>``, this will whitelist the written items and all the items in between  
  - Example: ``a\10 <...> a\15`` will whitelist ``a\10``, ``a\11``, ``a\12``, ``a\13``, ``a\14``, and ``a\15``  
### Assets
Some patches will not only inject code but also modify or add NARCs  
Any file in the ``Data`` folder will be copied to the filesystem in CTRMap when building
Using the ``-whitelist-assets`` or ``whitelist-all`` commands when building you can filter the files that will be copied using the ``Data\whitelist.txt`` file
### Globals
Sometimes code has to be shared between patches  
Any compilable file directly in the ``Globals`` will be compiled and added to any compiled patch  
### Patches
A folder in the ``Patches`` folder will be treated as a standalone patch  
``NOTE: Compilable files directly in the "Patches" folder will not be compiled``  
Each compilable file inside a patch folder (folder inside the ``Patches`` folder) will be compiled to a single patch  
``NOTE: You can create patches that hook to specific overlays so that they are only loaded when that overlay is``  
``NOTE: Make sure that any referenced or hooked game functions are in the "ESDB.yml" file``  
### Libraries
Each compilable file in the ``Libraries`` folder, including its subfolder, will be compiled as its own library  
Code that does not contain hooks to game functions can be loaded and unloaded at runtime using the functions in ``ExternalDependencies\NitroKernel\include\kDll.h``  
``NOTE: This is useful to reduce the size of patches``  
Using the ``-whitelist-libs`` or ``whitelist-all`` commands when building you can filter the folders inside ``Libraries`` that will be accesed to compile files using the ``Libraries\whitelist.txt`` file  
### ESDB
For the hooks and game functions to link back to the game code, you need a list that matches the functions with its location in RAM  
If you need to create a new ESDB from an IDB use this [guide](https://ds-pokemon-hacking.github.io/docs/generation-v/guides/bw_b2w2-code_injection/#symbol-maps)  
### Settings
The ``settings.h`` file is to be used to declare pre-compilation definitions that are used to enable or disable code sections and assets  
This is useful if you want to be able to activate and desavtivate features of your patch, if that is not the case you can leave it empty

### Functionality
Once your patch is done you have the following commands when running the program:  
- ``-install "Patch Dir"`` -> set up a build project
- ``-build "Patch Dir"`` -> build only the modified files in the patch
- ``-rebuild "Patch Dir"`` -> build the patch from scratch
  - ``-whitelist-libs`` -> ignore any folder in "Libraries" that is not specified in "Libraries/whitelist.txt"
  - ``-whitelist-assets`` -> ignore any file in "Assets" that is not specified in "Assets/whitelist.txt"
  - ``-whitelist-all`` -> activate all whitelist functionalities
  - ``-assets-override`` -> when a conflict appears when moving assets, the project asset gets automatically overriden
  - ``-assets-keep`` -> when a conflict appears when moving assets, the asset is not moved keeping the project asset
- ``-clear "Patch Dir"`` -> clear all build data (deletes "build" folder)
- ``-uninstall "Patch Dir"`` -> remove the patch completely from the CTRMap project
  - ``-keep-settings`` -> don't delete the build settings file when uninstalling
I recommend to uninstall before building after changing values in the ``settings.h`` file  
