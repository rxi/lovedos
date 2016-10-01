# Packaging
LoveDOS provides a built-in mechanism for packaging your game into an exe for
distribution. Simply pass `--pack` as the first argument to LoveDOS followed by
the your game's directory and the name of the output exe file.

For example, to package the game in the directory `mygame` to an exe file named
`mygame.exe`, you would run the following:
```batch
love --pack mygame mygame.exe
```

This would result in the file `mygame.exe` -- *Make sure to include the `.exe`
at the end of the name so LoveDOS knows to pack the project into an executable.*

You should also include `cwsdpmi.exe` with your game when distributing.
