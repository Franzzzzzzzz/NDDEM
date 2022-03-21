#!/bin/bash

if [ -e "../Samples" ]
then 
    echo -e "\nThe Samples file already exist and has been kept"
else
    while [ 1 ] ; do
    echo -e "\nA folder containing simulation results that Texturing can access is necessary. What type of folder do you want to create? \n [1] Regular folder (./Samples) \n [2] Symbolic link\n [3] Skip"
    read -p '>>>' -n 1 res
    
    if [ "$res" -eq 1 ]
    then 
        echo -e "\n./Samples will be created"
        mkdir Samples
        break 
    elif [ "$res" -eq 2 ]
    then 
        echo -e "\nWhere do you want the symlink to point to?"
        read -p '>>>' lnk 
        ln -s "$lnk" ./Samples
        break 
    elif [ "$res" -eq 3 ]
    then 
        break ;
    fi
    done
fi


if [ -e "../Textures" ]
then 
    echo -e "\nThe Textures file already exist and has been kept"
else
    while [ 1 ] ; do
    echo -e "\nA folder to hold the textures is required for Texturing. What type of folder do you want to create? \n [1] Regular folder (./Textures) \n [2] Symbolic link\n [3] Skip"
    read -p '>>>' -n 1 res
    
    if [ "$res" -eq 1 ]
    then 
        echo -e "\n./Textures will be created"
        mkdir "Textures"
        break 
    elif [ "$res" -eq 2 ]
    then 
        echo -e "\nWhere do you want the symlink to point to?"
        read -p '>>>' lnk 
        ln -s "$lnk" ./Textures
        break 
    elif [ "$res" -eq 3 ]
    then 
        break ;
    fi
    done
fi


