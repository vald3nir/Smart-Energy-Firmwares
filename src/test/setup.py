from subprocess import call


def load_libraries(libraries):
    call("pip install " + ' '.join(libraries), shell=True)


if __name__ == '__main__':
    load_libraries(
        libraries=[
            "setuptools", "wheel",
            "pyserial == 3.4",
            "matplotlib == 3.3.2"
        ]
    )
