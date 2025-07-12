# PW2Builder
Pokémon Generation 5 PMC code injection patch builder  

## How to install
``NOTE: Make sure you have both Git and Java installed in your computer``
- Install [Java](https://www.java.com/es/download/manual.jsp)
- Install [Git](https://git-scm.com/downloads)
- This builder attaches to your CTRMap project, if you don't already have one you will need to set up that first:
  - Install [CTRMap-CE](https://github.com/ds-pokemon-hacking/CTRMap-CE/releases)
  - Install [CTRMapV](https://github.com/ds-pokemon-hacking/CTRMapV/releases)
  - Create your CTRMap project
  - Install [PMC](https://github.com/ds-pokemon-hacking/PMC/releases) on to your project (you can follow this [guide](https://ds-pokemon-hacking.github.io/docs/generation-v/guides/bw_b2w2-code_injection/#setting-up-the-environment))
- Download the latest release of PW2Builder (if you are not using Windows you will have to build your own executable with CMake and put it in the ``Builder`` folder)
- Execute the ``ExternalDependencies\install_dependencies.bat`` file (if you are not in Windows you will need to change the name to ``install_dependencies.sh``)
- Download [ARM GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
- Open ``Builder\buildSettings.txt``
  - Write the path to your CTRMap project after ``SET PROJECT_DIR=`` (Ex. ``SET PROJECT_DIR=MyFolder\MyCTRMapProject\``)
  - Write the path to your CTRMap.jar file after ``SET CTRMAP_DIR=`` (Ex. ``SET CTRMAP_DIR=MyFolder\CTRMap\``)
  - Write the path to your ARM GNU Toolchain bin folder after ``SET ARM_NONE_EABI_DIR=`` (leave like it is if you set that folder in the systems path)
  - Write the path to your Java after ``SET JAVA_DIR=`` (leave like it is if you set that folder in the systems path)

## How to use
### Settings
The ``settings.h`` file is to be used to declare pre-compilation definitions that are used to enable or disable code sections and assets  
This is useful if you want to be able to activate and desavtivate features of your patch, if that is not the case you can leave it empty
### Create a patch
- Create a folder in the ``Patches`` folder with the name you want your patch to have  
``NOTE: Each folder in the "Patches" folder will compile all the compilable files (.cpp, .c and .S) into a .dll file``  
- Put the code to be injected in the folder you created  
``NOTE: You can add code that is used across multiple patches by putting that code inside the "Global" folder``  
- Make sure that any referenced or hooked game functions are in the ``ESDB.yml`` file
  - The release comes with the [PW2Code](https://github.com/Paideieitor/PW2Code) ESDB file
  - If you need to add new fuctions you can do so manually or create a new ESDB from an IDB using this [guide](https://ds-pokemon-hacking.github.io/docs/generation-v/guides/bw_b2w2-code_injection/#symbol-maps)
```
TIPS:
Any code that hooks functions from the game should be in a patch   
This are the folders that are automatically set as include directories:
  - ``PW2Builder``
    - ``ExternalDependencies``
        - ``swan``
        - ``NitroKernel\include``
        - ``libRPM\include``
        - ``ExtLib``
    - ``Global`
    - ``Headers``
```
### Add assets
Some patches will not only inject code but also modify or add NARCs  
Any file in the ``Data`` folder will be copied to the filesystem in CTRMap when building
Using the ``-whitelist-assets`` or ``whitelist-all`` commands when building you can filter the files that will be copied using the ``Data\whitelist.txt`` file
### Add libraries
Code that does not contain hooks to game functions can be loaded and unloaded at runtime using the functions in ``ExternalDependencies\NitroKernel\include\kDll.h``  
This is useful to reduce the default size of patches  
Each *.cpp* file in the ``Libraries`` folder, including its subfolder, will be compiled as its own *.dll* file  
Using the ``-whitelist-libs`` or ``whitelist-all`` commands when building you can filter the folders inside ``Libraries`` that will be accesed to compile files using the ``Libraries\whitelist.txt`` file
### Whitelist Sintax
Each whitelisted item needs to be in its own line in the file  
You can use pre-compiler conditionals using the definitions in the ``settings.h`` file to enable and disable the libraries compiled depending on the active features  
You can define a group by writting 2 items in the same line separated by ``<...>``, this will whitelist the written items and all the items in between  
  - Example: ``a\10 <...> a\15`` will whitelist ``a\10``, ``a\11``, ``a\12``, ``a\13``, ``a\14``, and ``a\15``
### Functionality
Once your patch is done you have the following commands when running the program:
  - ``-build``: build only the modified files in the patch
  - ``-rebuild``: build the patch from scratch
    - ``-whitelist-libs``: ignore any folder in ``Libraries`` that is not specified in ``Libraries\whitelist.txt``
    - ``-whitelist-assets``: ignore any file in ``Assets`` that is not specified in ``Assets\whitelist.txt``
    - ``-whitelist-all``: activate all whitelist functionalities
    - ``-assets-override``: when a conflict appears when moving assets, the project asset gets automatically overriden
    - ``-assets-keep``: when a conflict appears when moving assets, the asset is not moved keeping the project asset
  - ``-clear``: clear all build data (deletes ``build`` folder)
  - ``-uninstall``: remove the patch completely from the CTRMap project
I recommend to uninstall before building after changing values in the ``settings.h`` file
