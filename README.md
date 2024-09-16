# Other Engine

## Building

Only supports Windows at the moment

To build the engine, you need python 3.12+ installed and also the python3 OtherEngine toolset installed. To install the toolset run the following command from the root of the repo:

```bash
    python -m pip install ./tools
```
This will install the toolset which should now be available in your local python environment by using `import other`

With the toolchain installed, the cli tool should now work. 

To generate the project solution run the command

```bash
    python cli.py --generate-projects
```
or
```bash
    python cli.py -gp
```

You can now open `OtherEngine.sln` created by the first command and build in Visual Studio or you can run the build command:

```bash
    python cli.py --build
```
or
```bash
    python cli.py -b
```
> If the build fails try running it again with the verbose flag (`--verbose`/`-v`) to see the failure.

To do this in one command you can run the commands all at the same time:

```bash
    python cli.py -gp -b
```
or for verbose output
```bash
    python cli.py -v -gp -b
```

For more of the cli toolset commands run 
```bash
    python cli.py --help
```
or
```bash
    python cli.py -h
```

## Important Notes

- The engine is still in a very early stage and stability is not guaranteed, if you encounter any bugs, 
    feel free to open an issue or even submit a PR to fix it. 
- Full documentation for the OtherEngine python toolset is currently unwritten but is in progress as the toolset continues to develop.
