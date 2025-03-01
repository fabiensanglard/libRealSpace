# Quoi de neuf

- implémentation du system de script de missions
- implémentation du system de scene dans les missions
- implémentation de l'ia
- nouveau modèle de vol simplifié pour l'ia
- amélioration de la musique
- amélioration de l'utilisation des fonts
- amélioration de l'implémentation des scripts de gameflow
- création de l'écran de selection de l'armement
- création de l'écran de présentation du catalogue
- création de l'écran de présentation des comptes
- travaux de refactoring


# Missions

Les missions sont maintenant en partie jouable. Une partie du script de mission est décodé ce qui m'a permis de pouvoir implémenter une logique simple.

Pour y parvenir, j'ai longuement étudier les scripts de mission. J'ai réaliser un décodeur en python pour afficher l'information. Puis en comparant les actions des opcodes dans le jeux, j'ai pu implémenter la majeur partie des instructions. En parcourant la documentation de wing commander IV mise à disposition sur le site wing commander CIC, j'ai pu confirmer et infirmer certaine de mes hypothèses. Voici donc aujourd'hui le résultat.

# Refactoring / documentation

Cependant, avant d'implémenter les missions, j'ai fait quelques travaux de documentation. Oui, pour l'instant le projet étant toujours dans un stade "exploratoire", c'est un euphémisme de dire que c'est un peu le bazare. Pour y remédier, j'ai créér un grand schéma des différentes classes. Puis après j'ai procédé à quelques travaux de refactoring pour créer des entités dont l'objectif était plus clair. Avoir une meilleur séparation des responsabilités dans le code. Le travail n'est absolument pas terminé pour l'instant, mais c'est une première étape.

Toujours dans le refactoring, j'ai essayé de controler l'ia avec mon ancien code, mais la tache était très compliqué et me prenait trop de temp. Pendant ma phase d'exploration des script de missions, j'ai noté que dans le jeux original, les entité du jeux n'utilise pas le même modèle physique que le joueur, donc je me suis simplifié la tache en créans un modèle de vol très simple pour pouvoir controler les entités plus simplement. On pourra toujours plus tard revenir vers un meilleur modèle de vol et de meilleur controle.

# Jeux

Biensur le reste du jeux n'est pas oublié, j'ai commencé les écrans de selection des armes, du catalogue et du livre de compte. J'ai pu identifier de nouvelles instructions dans le script de gameflow, corrigé certains bugs. 

J'ai aussi travaillé sur la musique. Premièrement, j'ai une première conversion de la bank d'instrument ce qui nous offre une meilleur fidélité des musiques. Mais je suis tombé sur un bug dans la conversion de certains instruments, toutes les musiques ne sont pas ok.

# Prochaines étapes

En premier lieux, finir les écrans de selection de l'armement, de présentation des comptes et du catalogue. Une fois cela fait, je pourais implémenter les instructions la logique monétaire et donc le gameover lorsque les fonds sont négatifs.

J'aimerai aussi implémenter les écrans de fin de missions.

Dans le gameplay, il faut que j'améliore ma compréhension de la gestion des scenes pour pouvoir placer correctement les entités sur la cartes. Cela améliorera la fidélité des missions par rapport à l'original.
