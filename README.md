# Projet Quoridor

-- [Sujet du projet](https://www.labri.fr/perso/renault/working/teaching/projets/2020-21-S6-C-Quoridor.php)
 
-- [Page sur thor](https://thor.enseirb-matmeca.fr/ruby/projects/projetss6-quor)

# Documentation
- doc/html/index.html
# Executer le jeu

- GSL_PATH=/chemin/vers/gsl make
- make install
- LD_LIBRARY_PATH=/chemin/vers/gsl/lib ./install/server [-m <largeur du plateau>] [-t <type : c,t,h,s>] ./install/<client.so> ./install/<client.so>

# Executer les tests

- GSL_PATH=/chemin/vers/gsl make
- make test
- make install
- LD_LIBRARY_PATH=/chemin/vers/gsl/lib ./install/alltests

# Chemins à l'ENSEIRB

GSL_PATH=/net/ens/renault/save/gsl-2.6/install
LD_LIBRARY_PATH=/net/ens/renault/save/gsl-2.6/install/lib