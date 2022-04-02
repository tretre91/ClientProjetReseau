# Client

Client pour le projet de Réseau, peut se connecter en bluetooth au serveur correspondant (https://gitlab.dsi.universite-paris-saclay.fr/trevis.morvany/projetreseau)

## Build

Pour build utiliser cmake :

```bash
cmake -S . -B build -G "Unix Makefiles"
cd build
make client     # pour le client simple
make tui-client # pour le client graphique
```

## Usage

### Client simple (stable)

Pour lancer le client et se connecter au premier serveur disponible :

```bash
./client
```

Pour se connecter à un serveur en particulier :

```bash
./client adresse_du_serveur
```

on peut également spécifier le canal utilisé avec un 2ème argument.

Messages spéciaux :
- `stop` : pour se déconnecter
- `username nouveau_nom` : pour changer de nom d'utilisateur

### Client graphique (wip)

![](screenshot.png)

Utilisation :
- Lancer avec `./tui-client`
- Enter l'adresse bluetooth du serveur dans le champ "Adresse du serveur"
- Cliquer sur connexion
- Écrire un message dans la zone de texte du bas ("> ")
- Appuyer sur entrée pour envoyer

Messages spéciaux :
- `stop` : déconnexion (équivalent au bouton "Déconnexion")
- `!username nouveau_nom` : pour changer de nom d'utilisateur

Les navigations à la souris et au clavier sont supportées

Remarque : Il faut cliquer sur la zone des messages pour pouvoir scroller.
