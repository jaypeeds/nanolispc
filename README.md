# NANOLISP RETRO-COMPUTING

## Motivation

Dans les années 80, bien avant que cette tendance ne devienne possible économiquement pour le plus grand nombre,
certains passionnés ont entrevu les développements possibles d'une robotique personnelle.
C'est ainsi qu'est née la revue "Micros et Robots" dédiée à cette thématique. Sa vie fut brève, seulement 16 numéros
ont été publiés. En 1985, une série de trois articles sur le langage LISP sous la plume de JM Husson, commence au numéro 15. Quinze, seize, mais où est passé le troisième volet ? Cette revue était le rejeton d'une institution datant des
années 30, et c'est dans la revue mère, Radio-Plan, que le troisième volet est paru. On retrouve assez facilement des
scans de qualité suffisante de ces revues, sur le site "abandonware" ou sur "web.archive.org".
Le code est en Pascal, langage mainstream à cette époque, grâce à Apple et le système UCSD sur Apple II, puis plus tard
MacPascal ou TurboPascal sur PC.
Ma motivation est multiple, avec une grande curiosité pour commencer, et l'envie de comprendre, explorer, tester les
limites, peut-être les repousser.
LISP est avec FORTRAN l'un des plus anciens langages de programmation, datant du milieu du XXe siècle. Aujourd'hui,
il renaît en permanence, réinventé sur chaque écosystème productif, sur la JVM avec Clojure, sur la machine BEAM
d'Erlang avec LFE ou Clojerl, et même sur les micro-contrôleurs avec uLisp.com.   

## La démarche
Je me souviens avoir dactylographié tout le code lors de sa parution. Aujourd'hui, j'ai tenté une méthode plus efficace,
qui a plus ou moins réussi. Les listings ont été extraits par copie d'écran en séparant les colonnes publiées
côte-à-côte, puis transformés en PDF et déposés sur un Google drive. Le service Google permet de faire de l'OCR sur de tels documents, si on les ouvre en tant que Google Doc.
L'IDE de choix pour le Pascal, de nos jours, est Lazarus. Sur Mac, j'ai essayé, mais c'est assez compliqué à faire fonctionner, comme tous les outils en dehors de ceux d'Apple. Malgré les indications trouvées, malgré le certificat de signature du code, je n'ai pas réussi à passer la  sécurité qui assure que le débugger gdb ou lldb soit utilisé de manière sûre. Je me suis donc tourné vers Linux pour avancer plus rapidement.
Si on considère le fait que la plupart des implémentations initiales de Lisp ont été écrites en Assembleurs divers, Pascal, c'est presque de la triche ! Il existe des versions plus récentes, écrites en C, mais elles ont souvent une perspective moderne et avec la connaissance actuelle des langages de programmation (provenant de C++/Java). La démarche est différente lorsqu'on s'attache à suivre le chemin historique : Les S-Exp et leurs sept opérateurs quote, atom, eq, car, cdr, cons et cond.  

## Le bilan de la restauration
Il a fallu faire des ajustements, l'interpréte ne lisait pas la console mais un fichier source conventionnel SOURCE.NLSP. Mais ce n'est plus le cas depuis que la console es opérationnelle. La plupart des fonctions, notamment la création et la définition de fonctions par l'utilisateur, sont opérationnelles.
Les conventions de nommage ont été rendues plus strictes: Les primitives sont nommées F suivi de la primitive : f_car, f_cdr mais pour être exploitables, ces opérateurs devraient tous avoir un résultat de type S-Exp, ou SGRAPHE dans ce code. FNULL ne respectait pas cette convention et renvoyait un booléen du langage hôte inexploitable par l'interprète.

## Petit guide du langage
| Fonction | Description | Exemple | Code C |
|---------|-------|------|--------|
| CAR | Extrait le premier élément d'une liste | (CAR '(A B C)) --> A | [Sexp f_car(Sexp s);](https://github.com/jaypeeds/nanolispc/blob/01a8c48a98092db635071a66499a996343d8d1b8/NanoLisp.c#L165) |
| CDR | Extrait la sous-liste après le premier élément | (CDR '(A B C)) --> (B C) | [Sexp f_cdr(Sexp s);](https://github.com/jaypeeds/nanolispc/blob/01a8c48a98092db635071a66499a996343d8d1b8/NanoLisp.c#L177)
| CONS | Construit une liste, partant d'un atome et d'une liste. | (CONS 'A '(B C)) --> (A B C) | [Sexp f_cons(Sexp s1, Sexp s2);](https://github.com/jaypeeds/nanolispc/blob/01a8c48a98092db635071a66499a996343d8d1b8/NanoLisp.c#L240) |
| SETQ | Assigne une valeur à un atome. | (SETQ A 1) --> A vaut 1 | [Sexp f_setq(Sexp s);](https://github.com/jaypeeds/nanolispc/blob/01a8c48a98092db635071a66499a996343d8d1b8/NanoLisp.c#L208) |
| PRINT | Affiche la valeur d'un atome ou d'une liste | (PRINT A) --> Affiche 1, ne retourne rien| [Sexp f_print(Sexp s);](https://github.com/jaypeeds/nanolispc/blob/01a8c48a98092db635071a66499a996343d8d1b8/NanoLisp.c#L155) |
|ATOM| Test d'atomicité.| (ATOM 1) --> T, (ATOM (1 2)) --> ()|[Sexp f_atom(Sexp s);](https://github.com/jaypeeds/nanolispc/blob/01a8c48a98092db635071a66499a996343d8d1b8/NanoLisp.c#L196)|
|COND|Evaluation conditionnelle|(COND (L 'Non_Vide)(T 'Vide')) --> 'Non_Vide' si L est non vide, 'Vide sinon'|[Sexp f_cond(Sexp s);](https://github.com/jaypeeds/nanolispc/blob/01a8c48a98092db635071a66499a996343d8d1b8/NanoLisp.c#L257)|
|DE|Lie un nom à une définition de fonction.|(DE DUP(X)(CONS X (CONS X) ())), (DUP 1)--> (1 1)|[Sexp f_de(Sexp s);](https://github.com/jaypeeds/nanolispc/blob/01a8c48a98092db635071a66499a996343d8d1b8/NanoLisp.c#L225)|
|LOAD|Charge et interprète un fichier .nlsp|(LOAD 'Exemples/dbg-de.nlsp') --> Si le fichier ne se termine pas par (QUIT), rend la main à la console|[Sexp f_load(Sesxp s);](https://github.com/jaypeeds/nanolispc/blob/01a8c48a98092db635071a66499a996343d8d1b8/NanoLisp.c#L42)|
|OBLIST|Liste tous les atomes définis|(OBLIST) --> 	() T QUOTE CAR CDR CONS LAMBDA ATOM READ PRINT COND TRACE UNTRACE SETQ LOAD OBLIST QUIT|[void f_oblist(void)](https://github.com/jaypeeds/nanolispc/blob/91d7d358d2f6d6de99c6640a6f81e102870319d4/NanoLisp.c#L268)|
|TRACE|Active les trace de debug|(TRACE)|[n/a](n/a)|
|UNTRACE|Désactive les traces de debug|(UNTRACE)|[n/a](n/a)|
|QUIT|Quitte l'interprète Nano Lisp|(QUIT)|[n/a](n/a)|

## La TO DO list
- Implémenter l'arithmétique entière et réelle.
- Permettre la lecture à la console. (SETQ A (READ)) !
- Fonctions mathématiques et trigo en degrés.
- Boucles. Possibles grâce à EVAL. Voir Exemples/test-boucle.nlsp
- Liste de propriétés (property list). Voir Exemples/test-prop.nslp.
- Permettre des commentaires pour documenter le code. A faire !
- Gestion plus grâcieuse des erreurs. A faire !


## Comment étendre le langage
Pour assurer leur composabilité, dirait-on de nos jours, "monadique", toutes les fonctions exposables dans le langage doivent accepter des S-EXP en entrée et en sortie, le type Pascal SGRAPHE, elles sont nommées F-suivi du nom exposé: FCAR FCDR FCONS, etc.. Puis le nom exposé doit être ajouté dans la liste des tests, soit de EVAL, soit de APPLY. La fonction INIT permet d'enrichir le "dictionnaire" initial. La commande (OBLIST) permet de le lister.

## En guise de conclusion
Parfois, il faut envisager la création d'un petit langage en oubliant ce que l'on sait de nos jours sur la construction des compilateurs, l'analyse lexicale, la grammaire, la construction de l'arbre syntaxique, comme si le travail de Chomsky n'était pas encore connu... en se privant donc de toute une science dont les pionniers de l'informatique ne pouvaient disposer. En repartant des bases historiques, on redécouvre une forme de simplicité et son génie, qui permet (et personne ne s'en prive) de recréer en suivant ce parcours historique, un nouvel interprète LISP dans n'importe quel langage moderne.
