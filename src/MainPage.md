# Usage {#mainpage}

-- [Project's subject ](https://www.labri.fr/perso/renault/working/teaching/projets/2020-21-S6-C-Quoridor.php)
 
-- [Link to Thor](https://thor.enseirb-matmeca.fr/ruby/projects/projetss6-quor)

## Game's execution

- GSL_PATH=/chemin/vers/gsl make
- make install
- LD_LIBRARY_PATH=/chemin/vers/gsl/lib ./install/server [-m <largeur du plateau>] [-t <type : c,t,h,s>] ./install/<client.so> ./install/<client.so>

## Tests executions

- GSL_PATH=/chemin/vers/gsl make
- make test
- make install
- LD_LIBRARY_PATH=/chemin/vers/gsl/lib ./install/alltests

## Path to ENSEIRB

GSL_PATH=/net/ens/renault/save/gsl-2.6/install
LD_LIBRARY_PATH=/net/ens/renault/save/gsl-2.6/install/lib