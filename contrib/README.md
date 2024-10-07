# Contributed CLBOSS Utilities

## Installing

There are two ways to install the requirements:
- poetry
- nix

### Poetry
There are two ways to install poetry:
- pipx
- official installer

#### Pipx

```
# Install pipx
sudo apt update
sudo apt install pipx
pipx install poetry
```

#### [Or, click here for the official installer](https://python-poetry.org/docs/#installing-with-the-official-installer)

Once poetry is installed, install the Python dependencies:

```
# The following commands need to be run as the user who will be running
# the clboss utility commands (connecting to the CLN RPC port)

# Install clboss contrib utilities
poetry shell
poetry install
```

### Nix
If you have nix, you can just do, from the project root:
```
nix-shell contrib-shell.nix
```

Then before running the commands below, be sure to do:

```
cd contrib/
```

## Running

```
./clboss-earnings-history

./clboss-recent-earnings

./clboss-routing-stats

```
