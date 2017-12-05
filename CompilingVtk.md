# Compiling VTK #

## macOS Sierra ##

+ Checkout VTK master (as of DEC 2017, Probably wanting Vtk version 8.1)
+ I have the following VTK folder structure
	+ /Users/Shared/DREAM3D_SDK
		+ VTK
		+ VTK-Debug
		+ VTK-Release



    cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug  -DBUILD_TESTING=OFF -DVTK_Group_Qt=ON  -DVTK_QT_VERSION=5  -DQt5_DIR=/Users/Shared/DREAM3D_SDK/Qt5.9.2/5.9.2/clang_64/lib/cmake/Qt5 -DVTK_USE_SYSTEM_HDF5=ON  -DHDF5_Dir=/Users/Shared/DREAM3D_SDK/hdf5-1.8.19-Debug/share/cmake -DHDF5_C_INCLUDE_DIR:PATH=/Users/Shared/DREAM3D_SDK/hdf5-1.8.19-Debug/include -DHDF5_hdf5_LIBRARY_DEBUG:FILEPATH=/Users/Shared/DREAM3D_SDK/hdf5-1.8.19-Debug/lib/libhdf5_debug.1.8.19.dylib -DHDF5_hdf5_LIBRARY_RELEASE:FILEPATH=/Users/Shared/DREAM3D_SDK/hdf5-1.8.19-Release/lib/libhdf5.1.8.19.dylib -DHDF5_hdf5_hl_LIBRARY_DEBUG:FILEPATH=/Users/Shared/DREAM3D_SDK/hdf5-1.8.19-Debug/lib/libhdf5_hl_debug.1.8.19.dylib -DHDF5_hdf5_hl_LIBRARY_RELEASE:FILEPATH=/Users/Shared/DREAM3D_SDK/hdf5-1.8.19-Release/lib/libhdf5_hl.1.8.19.dylib VTK_USE_SYSTEM_HDF5:BOOL=ON ../VTK
    
    
    
## Windows MSVC ##





## Linux ##


