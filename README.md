# GW2TacO | Tactical Overlay for Guild Wars 2

## Description
TacO is an overlay for Guild Wars 2 which has many cool features like a maptimer but most importantly allows you to create your own markers for map completion, jumping puzzles, festivals etc.   

## Download for Usage
If you want to use TacO like a normal person and don't want to contribute please visit http://www.gw2taco.com/ and click on "DOWNLOAD the latest build".

## Contributing
If you want to take part in like fixing issues or contributing directly to the code base, plase read the following part.

### Prerequisites
- Visual Studio (preferebly 2019)
- June 2010 DirectX SDK

### Contributing
1. Create a fork and download the repository 
```bash
git clone <forked repo>
``` 

### Testing
1. Start Guild Wars 2 and log onto a character
2. Open the GW2TacO.sln with Visual Studio
3. In the toolbar on the top left click on "Local Windows Debugger"
4. You should now be able to debug TacO ingame in real time. 

### Building a Release
1. Open the GW2TacO.sln file with Visual Studio
2. On the toolbar on the top left select "**Release**" and "**x32**" 
3. Go to the right side and rightclick "GW2TacO" under the _Solution explorer_ and press **Build** (if you already build the project press **Rebuild**
4. Now a new folder called _**Release/**_ should exist with the builded files in it

<hr>

[Issues][github-issues] - 
[Pull requests][github-pulls]

<hr>  

###### Licensed under the [Attribution-NonCommercial 4.0 International License][github-license].

[github-license]: https://github.com/BoyC/GW2TacO/blob/main/LICENSE
[github-issues]: https://github.com/BoyC/GW2TacO/issues
[github-pulls]: https://github.com/BoyC/GW2TacO/pulls