# Other Engine

## Building

Only supports Windows at the moment

To build the engine, you need python 3+ installed and then from the root of the repo run the following commands:

```bash
    python3 cli.py gensln
    python3 cli.py buildsln
```
> you can also open `OtherEngine.sln` created by the first command and build in Visual Studio

## Important Notes

- The engine is still in a very early stage and stability is not gauranteed, if you encounter any bugs, feel free to open an issue or even submit a PR to fix it. 
- The launcher only has the capability to open existing projects (as long as they exists in it's project.cfg file). Project creation is coming soon. There will be instructions for
    creating a project manually soon in the meantime.
