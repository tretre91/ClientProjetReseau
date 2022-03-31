# Client

## Build

Pour build utiliser cmake :

```bash
cmake -S . -B build -G "Unix Makefiles"
cd build
make client
```

## Usage

Pour lancer le client :

```bash
./client <adresse du serveur>
```

Messages spéciaux :
- `stop` : pour se déconnecter
- `username nouveau_nom` : pour changer de nom d'utilisateur
