from project.project_builders import full_build

if __name__ == "__main__":
    try:
        print("> performing full rebuilding")
        res = full_build()
        if res == 0:
            print(" > rebuild successful")
        else:
            print(" !> rebuild failed!")

    except EnvironmentError:
        print("Other Engine does not yet support anything but windows!")
