#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <MLV/MLV_all.h>

#define SIZE_X 700
#define SIZE_Y 800
#define CLIC_DROIT clicSouris == MLV_BUTTON_RIGHT && etatBouton == MLV_RELEASED
#define CLIC_GAUCHE clicSouris == MLV_BUTTON_LEFT && etatBouton == MLV_RELEASED

typedef struct _game {
    int width;
    int height;
    int mines;
    int **terrain;
    int termine;
} Game;

// Fonctions construction
int initFichier(FILE *cheminFichier, Game *g);
void sauvFichier(Game g, char *mode);
int **alloc_terrain(int hauteur, int largeur);
void libere_terrain(Game *g);
void randTerrain(int **terrain, int a, int h, int l, int m);
int recupArguments(int argc, char **argv, Game *g, int *seed, int *h, int *l, int *m, int *sauvegarde);
void dessineGrille(Game *g, double *size_x, double *size_y, double *grille_x, double *grille_y, int premiereExec);
void clic(Game *g, double grille_x, double grille_y, int sauvegarde, int *rejouer, int *perdu, int *quitter);

// Fonctions du jeu (PL et autres)
void print_g(Game *g);
int nbmines_g(Game *g, int i, int j);
int hasmine_g(Game *g, int i, int j);
int victoire_g(Game *g);
void Drapeau_g(Game *g, int i, int j);
int Pied_g(Game *g, int i, int j);
int dans_grille(Game g, int i, int j);
void explose(Game *g, int i, int j, int nouvEtat);

int main(int argc, char **argv){
    int h = 0;
    int l = 0;
    int m = 0;
    int seed;
    int sauvegarde = 0;                                                                                                                                                                   
    Game g;
    int perdu = 0;
    int rejouer = 1;
    int quitter = 0;
    g.termine = 0;

    double size_x, size_y, grille_x, grille_y;

    srand(time(NULL));
    
    if (recupArguments(argc, argv, &g, &seed, &h, &l, &m, &sauvegarde)){
        if (h != 0 && l != 0 && m != 0){
            g.height = h;
            g.width = l;
            g.mines = m;
            g.terrain = alloc_terrain(h, l);

            randTerrain(g.terrain, seed, g.height, g.width, g.mines);
        }

        sauvFichier(g, "./copie.txt"); // Copie pour la sauvegarde

        printf("h: %d, l: %d, m: %d, seed: %d\n", g.height, g.width, g.mines, seed);

        print_g(&g);

        dessineGrille(&g, &size_x, &size_y, &grille_x, &grille_y, 1);
        
        while(rejouer && !quitter){
            clic(&g, grille_x, grille_y, sauvegarde, &rejouer, &perdu, &quitter);
            
            if (perdu || victoire_g(&g)){
                g.termine = 1;
                rejouer = 0;
                dessineGrille(&g, &size_x, &size_y, &grille_x, &grille_y, 0);
                clic(&g, grille_x, grille_y, sauvegarde, &rejouer, &perdu, &quitter);
            }

            dessineGrille(&g, &size_x, &size_y, &grille_x, &grille_y, 0);
        }

        libere_terrain(&g);
        MLV_free_window();
    }
    else{
        printf("Utilisation: ./minesweeper [-a seed][-j h l nbMines][-u][filename]\nAvec nbMines < h*l\n");
    }

    return 0;
}

// Crée le tableau à partir d'un fichier
int initFichier(FILE *cheminFichier, Game *g){
    fscanf(cheminFichier, "%d", &(g->height));
    fscanf(cheminFichier, "%d", &(g->width));
    fscanf(cheminFichier, "%d", &(g->mines));

    if (g->mines < g->width*g->height){
        g->terrain = alloc_terrain(g->height, g->width);

        for (int i = 0; i < g->height; i++){
            for (int j = 0; j < g->width; j++){
                fscanf(cheminFichier, "%d", &(g->terrain[i][j]));
            }
        }

        fclose(cheminFichier);
        return 1;
    }

    fclose(cheminFichier);
    return 0;
}

void sauvFichier(Game g, char *mode){
    FILE *copie = fopen(mode, "w");
    
    fprintf(copie, "%d", g.height);
    fputc(' ', copie);
    fprintf(copie, "%d", g.width);
    fputc(' ', copie);
    fprintf(copie, "%d", g.mines);
    for(int i = 0 ; i < g.height; i++){
        fputc('\n', copie);
        for(int j = 0; j < g.width; j++){
            fprintf(copie, "%d", g.terrain[i][j]);
            fputc(' ', copie);
        }
    }

    fclose(copie);
}

int **alloc_terrain(int hauteur, int largeur){
    int **terrain = malloc(sizeof(int *)*hauteur);
    
    for(int i=0;i<hauteur;i++){
        terrain[i] = malloc(sizeof(int)*largeur);
    }

    return terrain; 
}

void libere_terrain(Game *g){
    for (int i = 0; i < g->height; i++){
        free(g->terrain[i]);
    }
    free(g->terrain);
}

void print_g(Game *g){
    int height = g->height;
    int width = g->width;

    for (int i = 0; i < height; i++){
        for (int j = 0; j < width; j++){
            printf("%d ", g->terrain[i][j]);
        }
        printf("\n");
    }
}

void randTerrain(int **terrain, int a, int h, int l, int m){
    int x, y;

    // Pose des mines aléatoirement
    int cpt = 0;
    while(cpt != m){
        x = rand() % h; // indice entre 0 et h-1
        y = rand() % l; // indice entre 0 et l-1

        if (terrain[x][y] != 9){
            terrain[x][y] = 9; // pose une mine
            cpt += 1;
        }
    }

    for (int i = 0; i < h; i++){
        for (int j = 0; j < l; j++){
            if (terrain[i][j] != 9){
                terrain[i][j] = 0;
            }
        }
    }
}

int recupArguments(int argc, char **argv, Game *g, int *seed, int *h, int *l, int *m, int *sauvegarde){
    if (argc == 2){
        FILE *fichier = fopen(argv[1], "r");
        if (fichier != NULL){
            return initFichier(fichier, g);
        }
        else{
            printf("aA\n");
           *seed = rand() % 1001; // seed max 1000
           *h = 10;
           *l = 10;
           *m = 10; 
        }
    }
    else if (argc == 3){
        if (argv[1][0] == '-'){
            if (argv[1][1] == 'a'){
                *seed = atoi(argv[2]);
                *h = 10;
                *l = 10;
                *m = 10;
            }
            else if (argv[1][1] == 'u'){
                *sauvegarde = 1;
                
                FILE *fichier = fopen(argv[2], "r");
                if (fichier != NULL){
                    return initFichier(fichier, g);
                }
                else{
                   *seed = rand() % 1001;
                   *h = 10;
                   *l = 10;
                   *m = 10; 
                }
            }
        }
    }
    else if (argc == 4){
        if (argv[3][0] == '-'){
            *sauvegarde = 1;
            *seed = atoi(argv[2]);
            *h = 10;
            *l = 10;
            *m = 10;
        }
        else{  
            FILE *fichier = fopen(argv[3], "r");
            if (fichier != NULL){
                return initFichier(fichier, g);
            }
            else{
               *seed = atoi(argv[2]);
               *h = 10;
               *l = 10;
               *m = 10; 
            }
        } 
    }
    else if (argc == 5){
        *sauvegarde = 1;

        FILE *fichier = fopen(argv[4], "r");
        if (fichier != NULL){
            return initFichier(fichier, g);
        }
        else{
           *seed = atoi(argv[2]);
           *h = 10;
           *l = 10;
           *m = 10; 
        }    
    }
    else if (argc == 7){
        *seed = atoi(argv[2]);
        *h = atoi(argv[4]);
        *l = atoi(argv[5]);
        *m = atoi(argv[6]);

        if (*m >= (*h)*(*l)){
            return 0;
        }
    }
    else if (argc == 8){
        if (argv[7][0] == '-'){
            *sauvegarde = 1;
            *seed = atoi(argv[2]);
            *h = atoi(argv[4]);
            *l = atoi(argv[5]);
            *m = atoi(argv[6]);

            if (*m >= (*h)*(*l)){
                return 0;
            }
        }
        else{  
            FILE *fichier = fopen(argv[7], "r");
            if (fichier != NULL){
                return initFichier(fichier, g);
            }
            else{
               *seed = atoi(argv[2]);
               *h = atoi(argv[4]);
               *l = atoi(argv[5]);
               *m = atoi(argv[6]);
            }
        } 
    }
    else if (argc == 9){
        *sauvegarde = 1;

        FILE *fichier = fopen(argv[4], "r");
        if (fichier != NULL){
            return initFichier(fichier, g);
        }
        else{
           *seed = atoi(argv[2]);
           *h = atoi(argv[4]);
           *l = atoi(argv[5]);
           *m = atoi(argv[6]);

            if (*m >= (*h)*(*l)){
                return 0;
            }
        }     
    }
    else{
        return 0;
    }

    return 1;
}

void dessineGrille(Game *g, double *size_x, double *size_y, double *grille_x, double *grille_y, int premiereExec){  
    if(premiereExec){
        if (g->height == g->width){
            *size_x = SIZE_X;
            *size_y = SIZE_Y;
            *grille_x = *size_x;
            *grille_y = *size_y;
        }
        else if (g->height < g->width){
            *size_x = SIZE_X;
            *size_y = g->height * ((*size_x)/g->width) + 100;
            *grille_x = *size_x;
            *grille_y = *size_y;
        }
        else if (g->height > g->width){
            *size_y = SIZE_Y;
            *grille_y = *size_y;
            *size_x = g->width * ((*(grille_y) - 100)/g->height);
            *grille_x = *size_x;
        }
    
        MLV_create_window("Démineur", "Démineur", *size_x, *size_y);
    }

    MLV_Image *img_recommencer = MLV_load_image("./images/recommencer.png");
    MLV_Image *img_quitter = MLV_load_image("./images/quitter.png");
    MLV_Image *img_case_nd = MLV_load_image("./images/case_non_decouverte.png");
    MLV_Image *img_case_d = MLV_load_image("./images/case_decouverte.png");
    MLV_Image *img_drapeau = MLV_load_image("./images/drapeau.png");
    MLV_Image *img_mine_g = MLV_load_image("./images/mine.png");
    MLV_Image *img_mine_r = MLV_load_image("./images/mine_expl.png");
    MLV_Image *img_1 = MLV_load_image("./images/1.png");
    MLV_Image *img_2 = MLV_load_image("./images/2.png");
    MLV_Image *img_3 = MLV_load_image("./images/3.png");
    MLV_Image *img_4 = MLV_load_image("./images/4.png");
    MLV_Image *img_5 = MLV_load_image("./images/5.png");
    MLV_Image *img_6 = MLV_load_image("./images/6.png");
    MLV_Image *img_7 = MLV_load_image("./images/7.png");
    MLV_Image *img_8 = MLV_load_image("./images/8.png");

    MLV_resize_image(img_recommencer, (*size_x)/2, 100);
    MLV_resize_image(img_quitter, (*size_x)/2, 100);
    MLV_draw_image(img_recommencer, 0, 0);
    MLV_draw_image(img_quitter, (*size_x)/2, 0);
    

    double lrgCase = *grille_x/g->width;
    double hautCase = (*(grille_y)-100)/g->height;
    
    MLV_resize_image(img_case_nd, lrgCase, hautCase);
    MLV_resize_image(img_case_d, lrgCase, hautCase);
    MLV_resize_image(img_drapeau, lrgCase, hautCase);
    MLV_resize_image(img_mine_g, lrgCase, hautCase);
    MLV_resize_image(img_mine_r, lrgCase, hautCase);
    MLV_resize_image(img_1, lrgCase, hautCase);
    MLV_resize_image(img_2, lrgCase, hautCase);
    MLV_resize_image(img_3, lrgCase, hautCase);
    MLV_resize_image(img_4, lrgCase, hautCase);
    MLV_resize_image(img_5, lrgCase, hautCase);
    MLV_resize_image(img_6, lrgCase, hautCase);
    MLV_resize_image(img_7, lrgCase, hautCase);
    MLV_resize_image(img_8, lrgCase, hautCase);

    double posCase_x = 0;
    double posCase_y = 100;
    for (int i = 0; i < g->height; i++){
        for (int j = 0; j < g->width; j++){ 
            MLV_draw_rectangle(posCase_x, posCase_y, lrgCase, hautCase, MLV_COLOR_WHITE);

            // Case non découverte
            if ((g->terrain[i][j] == 0) || (g->terrain[i][j] == 9 && !g->termine)){
                MLV_draw_image(img_case_nd, posCase_x, posCase_y);
            }
            else if(g->terrain[i][j] == 9 && g->termine){
                MLV_draw_image(img_case_d, posCase_x, posCase_y);
                MLV_draw_image(img_mine_g, posCase_x, posCase_y);
            }
            // Case non découverte avec drapeau
            else if (g->terrain[i][j] == -9){
                if (g->termine){
                    MLV_draw_image(img_case_d, posCase_x, posCase_y);
                    MLV_draw_image(img_mine_g, posCase_x, posCase_y);
                }
                else{
                    MLV_draw_image(img_case_nd, posCase_x, posCase_y);
                }
                MLV_draw_image(img_drapeau, posCase_x, posCase_y);
            }
            else if (g->terrain[i][j] == -10){
                MLV_draw_image(img_case_nd, posCase_x, posCase_y);
                MLV_draw_image(img_drapeau, posCase_x, posCase_y);
            }
            else if(g->terrain[i][j] == 1){
                MLV_draw_image(img_case_d, posCase_x, posCase_y);
                MLV_draw_image(img_1, posCase_x, posCase_y);
            }
            else if(g->terrain[i][j] == 2){
                MLV_draw_image(img_case_d, posCase_x, posCase_y);
                MLV_draw_image(img_2, posCase_x, posCase_y);
            }
            else if(g->terrain[i][j] == 3){
                MLV_draw_image(img_case_d, posCase_x, posCase_y);
                MLV_draw_image(img_3, posCase_x, posCase_y);
            }
            else if(g->terrain[i][j] == 4){
                MLV_draw_image(img_case_d, posCase_x, posCase_y);
                MLV_draw_image(img_4, posCase_x, posCase_y);
            }
            else if(g->terrain[i][j] == 5){
                MLV_draw_image(img_case_d, posCase_x, posCase_y);
                MLV_draw_image(img_5, posCase_x, posCase_y);
            }
            else if(g->terrain[i][j] == 6){
                MLV_draw_image(img_case_d, posCase_x, posCase_y);
                MLV_draw_image(img_6, posCase_x, posCase_y);
            }
            else if(g->terrain[i][j] == 7){
                MLV_draw_image(img_case_d, posCase_x, posCase_y);
                MLV_draw_image(img_7, posCase_x, posCase_y);
            }
            else if(g->terrain[i][j] == 8){
                MLV_draw_image(img_case_d, posCase_x, posCase_y);
                MLV_draw_image(img_8, posCase_x, posCase_y);
            }
            else if(g->terrain[i][j] == 10){
                MLV_draw_image(img_case_d, posCase_x, posCase_y);
                MLV_draw_image(img_mine_r, posCase_x, posCase_y);
            }
            else if(g->terrain[i][j] == -11){
                MLV_draw_image(img_case_d, posCase_x, posCase_y);
            }

            posCase_x += lrgCase;
        }
        posCase_x = 0;
        posCase_y += (*(grille_y)-100) / g->height;
    }

    MLV_actualise_window();
    
    MLV_free_image(img_recommencer);
    MLV_free_image(img_quitter);
    MLV_free_image(img_case_nd);
    MLV_free_image(img_case_d);
    MLV_free_image(img_drapeau);
    MLV_free_image(img_mine_g);
    MLV_free_image(img_mine_r);
    MLV_free_image(img_1);
    MLV_free_image(img_2);
    MLV_free_image(img_3);
    MLV_free_image(img_4);
    MLV_free_image(img_5);
    MLV_free_image(img_6);
    MLV_free_image(img_7);
    MLV_free_image(img_8);
}

void clic(Game *g, double grille_x, double grille_y, int sauvegarde, int *rejouer, int *perdu, int *quitter){
    int x, y;
    double hautCase = (grille_y - 100) / g->height;
    double largCase = grille_x / g->width;
    
    MLV_Event event;
    MLV_Mouse_button clicSouris;
    MLV_Button_state etatBouton;
    
    do{
        event = MLV_wait_event(NULL, NULL, NULL, NULL, NULL, &x, &y, &clicSouris, &etatBouton);
        if((g->termine == 1 && y >= 100) || (g->termine && clicSouris == MLV_BUTTON_RIGHT)){
            event = MLV_NONE;
        }
    } while (event != MLV_MOUSE_BUTTON);

    int i = 0;
    int j = 0;
    // Récupère l'indice i de la case où l'utilisateur a cliqué
    for (i = 0; (i <= g->height) && ((fabs(hautCase*i - (y-100))) > hautCase); i++);
    // Récupère l'indice j de la case où l'utilisateur a cliqué
    for (j = 0; (j <= g->width) && ((fabs(largCase*j - x)) > largCase); j++);
    
    // Pose pied
    if (clicSouris == MLV_BUTTON_LEFT && etatBouton == MLV_RELEASED && y >= 100 && !g->termine){
        if (Pied_g(g, i, j)){
            *perdu = 1;
            printf("Vous avez perdu!\n");
        }
    }
    // Pose drapeau
    else if(clicSouris == MLV_BUTTON_RIGHT && etatBouton == MLV_RELEASED){
        Drapeau_g(g, i, j);
    }
    // Boutons recommencer et quitter
    else if (clicSouris == MLV_BUTTON_LEFT && y < 100){
        if (x < grille_x/2){
            FILE *copie = fopen("./copie.txt", "r");
            libere_terrain(g);
            initFichier(copie, g);
            *rejouer = 1;
            g->termine = 0;
            *perdu = 0;
        }
        else if(x > grille_x/2){
            *quitter = 1;
        }
    }

    if (sauvegarde){
        sauvFichier(*g, "./sauvegarde.txt");
    }
}

int dans_grille(Game g, int i, int j){
    return ((i >= 0 && i < g.height) && (j >= 0 && j < g.width));
}

void explose(Game *g, int i, int j, int nouvEtat){
    int precEtat = g->terrain[i][j];

    if (precEtat == nouvEtat){
        return;
    }
    g->terrain[i][j] = nouvEtat;
    
    int voisins[8][2] = {{i-1, j-1}, {i-1, j}, {i-1, j+1}, {i, j-1}, {i, j+1}, {i+1, j-1}, {i+1, j}, {i+1, j+1}};
    
    for (int k = 0; k < 8; k++){
        int voisin_i = voisins[k][0];
        int voisin_j = voisins[k][1];
        if (dans_grille(*g, voisin_i, voisin_j) && g->terrain[voisin_i][voisin_j] == precEtat){
            if(nbmines_g(g, voisin_i, voisin_j) == 0){
                explose(g, voisins[k][0], voisins[k][1], nouvEtat);
            }
            else{
                Pied_g(g, voisin_i, voisin_j);
            }
        }
    }
}

int nbmines_g(Game *g, int i, int j){
    int cpt = 0;

    // Boucle commencant par la case en haut à gauche de (i, j) et passe par toutes les cases entourant (i, j)
    // jusqu'à la case qui est en bas à droite.
    for (int k = i-1; k <= i+1; k++){
        for (int l = j-1; l <= j+1; l++){
            // Ignore la case (i, j)
            if (k != i || l != j){
                // La fonction hasmine_t s'occupe déjà de vérifier si i et j ne sont pas hors du terrain
                cpt += hasmine_g(g, k, l);
            }
        }
    }

    return cpt;
}

int hasmine_g(Game *g, int i, int j){
    int height = g->height;
    int width = g->width;
    
    // Si les coordonnées sortent du terrain
    if (i < 0 || i >= height || j < 0 || j >= width){
        return 0;
    }

    // Case non découverte avec mine, case avec drapeau et mine
    if (g->terrain[i][j] == 9 || g->terrain[i][j] == -9){
        return 1;
    }
    
    // Case non découverte sans mines, case découverte sans mines, case sans mines avec drapeau
    return 0;
}

int victoire_g(Game *g){
    int cptMines = 0;
    int cptDrapeau = 0; // Compteur pour les drapeaux bien placés
    int mauvaisDrapeau = 0; // Booléen pour indiquer si il y a un drapeau mal placé
    int nonDecouvert = 0; // Booléen pour indiquer si il y a une seule case non découverte restante
    int height = g->height;
    int width = g->width;

    for (int i = 0; i < height; i++){
        for (int j = 0; j < width; j++){
            // Compte le nombre de mines manuellement vu l'erreur sur PL
            cptMines += hasmine_g(g, i, j);
            
            // Teste si il y a une seule case non découverte sans mines
            if ((g->terrain[i][j] == 0 || g->terrain[i][j] == -10) && nonDecouvert == 0){
                nonDecouvert = 1;
            }
            // Teste si il y a un drapeau mal placé
            else if (g->terrain[i][j] == -10 && !mauvaisDrapeau) { mauvaisDrapeau = 1; }
            // Sinon, compte le nombre de drapeaux bien placées
            else if (g->terrain[i][j] == -9 && !mauvaisDrapeau) { cptDrapeau += 1; }
        }
    }
    // Si il y a autant de drapeaux bien placés que de mines, c'est une victoire
    if ((cptDrapeau == cptMines) && !mauvaisDrapeau) { return 1; }
    // Si il n'y a aucune case (sans mines) non découverte, c'est une victoire
    if (nonDecouvert == 0) { return 1; }
    // Si les deux scénarios ci-dessus sont faux, alors ce n'est pas encore une victoire
    return 0;
}

void Drapeau_g(Game *g, int i, int j){
    int height = g->height;
    int width = g->width;
    
    // Si les coordonnées sortent du terrain, ne fais rien
    if (i < 0 || i >= height || j < 0 || j >= width);
    
    // Non découvert sans mines sans drapeau --> Non découvert sans mines avec drapeau
    else if (g->terrain[i][j] == 0) { g->terrain[i][j] = -10; }
    // Non découvert sans mines avec drapeau --> Non découvert sans mines sans drapeau 
    else if (g->terrain[i][j] == -10) { g->terrain[i][j] = 0; }
    // Non découvert avec mines sans drapeau --> Non découvert avec mines avec drapeau
    else if (g->terrain[i][j] == 9) { g->terrain[i][j] = -9; }
    // Non découvert avec mines avec drapeau --> Non découvert avec mines sans drapeau
    else if (g->terrain[i][j] == -9) { g->terrain[i][j] = 9; }
}

int Pied_g(Game *g, int i, int j){
    
    // Non découvert sans mines
    if (g->terrain[i][j] == 0){ 
        int nbMines = nbmines_g(g, i, j);
        
        if (nbMines == 0){ explose(g, i, j, -11); } // Si il n'y a pas de mines à proximité
        else{ g->terrain[i][j] = nbMines; }
        
        return 0;
    }
    // Non découvert avec mine
    else if (g->terrain[i][j] == 9){
        g->terrain[i][j] = 10;
        return 1;
    }

    // Dans tous les autres cas, on n'explose pas
    return 0;
}