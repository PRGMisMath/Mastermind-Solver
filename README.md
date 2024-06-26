# Mastermind-Solver
Ce programme permet de générer la "solution" du Mastermind en utilisant la théorie de l'information de Shannon : c'est à dire la suite d'instruction à suivre pour gagner en le moins de tour possible.

## Comment utiliser le programme ?
Avant de compiler le code, il faut modifier les macros :
 - `NB_CAR` : pour changer le nombre de pions
 - `NB_COUL` : pour changer le nombre de couleur

Il faut aussi mettre à jour les autres macros :
 - `NB_COMBIS` : qui vaut NB_COUL ^ NB_CAR
 - `NB_ANSW` : qui vaut (NB_CAR + 1) * (NB_CAR + 2) / 2

et aussi vérifier que la liste `color_list` a bien le bon nombre d'éléments.

Ensuite, il ne vous reste plus qu'à compiler et à lancer le programme.

## Comment exploiter les résultats ?
La macro `JS_SAVE` permet de changer de mode d'enregistrement des données :
 - Si elle est activée, le résultat sera donnée sous la forme d'une liste de liste qui est exploitable par l'interpréteur Python ou JavaScript. C'est ce mode là qui est employée pour faire fonctionner la page Web présente dans `/tools`.
 - Sinon, le résulat sera donnée sous une forme 'lisible' par un humain comme c'est le cas dans `/results`.

## Conseils
Si votre ordinateur n'est pas très puissant, il est conseillé d'éviter de rentrer des trop gros nombres pour `NB_CAR` et `NB_COMBIS` et aussi de définir la macro `NO_LOG` qui désactive l'affichage d'informations durant les calculs.

## Détails
La complexité de l'algorithme est située entre N^2 et N^3 (avec N pour NB_COMBIS).
