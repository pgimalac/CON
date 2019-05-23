# Informations générales
Le projet s'appelle CON pour C Othello Network (evidemment).

Il s'agit d'un projet personnel commencé en septembre 2016 pour m'entraîner en C et m'initier à la programmation réseau.

Il se trouve que la V1.0 est le projet de C de 2015-2016 (je m'en suis rendu compte après coup).

## Compilation et exécution
Un makefile est fourni, un simple `make all` ou `make re` compilera la totalité du projet.

La bibliothèque graphique SDL (version 2) est nécessaire à l'exécution du programme.

Pour l'exécuter en réseau, il faut ouvrir trois terminaux et faire :
- Dans le premier terminal, `./con server`
- Dans le deuxième `./con host` puis donner son pseudo
- Dans le troisième, `./con client` puis choisir l'hôte avec le bon nom
En local un simple `./con local` suffit.

Le premier terminal affichera des informations pratiques pour la connexion, si tout se passe bien il ne sert visuellement à rien.

### V1.0
Version finie.

##### Particularités
- [x] Jeu à deux joueurs humains sur la même machine
- [x] Interface graphique
- [x] Contrôles à la souris sur l'interface
- [x] Affichage du joueur actuel
- [x] Boutons pour quitter, recommencer
- [x] Annulation des coups

### V2.0
Version en cours.

##### Particularités
- [x] Possibilité de jouer en réseau local sur deux machines distinctes
- [x] Utilisation d'un server pour faire le lien entre les clients
- [x] Possibilité de choisir son adversaire en indiquant son IP
- [x] Menu de matchmaking pour choisir son adversaire

### V3.0
Future version. Pour l'instant théorique.

##### Particularités
- [ ] IA basique
- [ ] "Bonne" IA
- [ ] IA réglable, le meilleur niveau étant très difficilement battable.
