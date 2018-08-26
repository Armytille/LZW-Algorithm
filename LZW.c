#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define _TAILLE_MAX_DICO UINT16_MAX

/*Le dictionnaire. */
uint8_t ** dico;
/*L'indice auquel on ajoute les mots dans le dictionnaire, incrémenté au fur et à mesure des ajouts.*/
uint16_t indice_fin_dico = 256;

/*Fonction qui renvoit 1 si la chaîne de caractère passée en paramètre est déjà contenue dans le dictionnaire, 0 sinon.*/
uint8_t appartient_au_dico(uint8_t *w){
	int i;

	for(i=0;i<indice_fin_dico;i++)
		if(!strcoll((const char*)dico[i],(const char*)w))
			return 1;
	return 0;
}

/*Fonction qui écrit en binaire dans le fichier l'indice de la chaîne de caractère passée en argument sur 16 bits sur la plupart des machines.*/
void donner_l_indice_de_w(FILE * fichier,uint8_t *w){
	uint16_t i;

	if(w != NULL)
		for(i=0;i<indice_fin_dico;i++)
			if(dico[i] != NULL && !strcoll((const char*)dico[(int)i],(const char*)w)){
				fwrite(&i, sizeof(i),1,fichier);
				break;
			}
}

/*Fonction qui lit dans le fichier texte caractère par caractère, ajoute les mots au dictionnaire et écrit le code compressé au fur et à mesure de la lecture.*/
void LZW(FILE * fichier, FILE * fichier_dico){
	uint8_t *w=NULL, *tmp=NULL, str[2],c,taille = 20;

	if((w = malloc(taille*sizeof(uint8_t))) == NULL)
		exit(EXIT_FAILURE);
	if((tmp = malloc(taille*sizeof(uint8_t))) == NULL)
		exit(EXIT_FAILURE);

	while((int8_t)(c = fgetc(fichier)) != EOF){
		str[0] = c;
		str[1] = '\0';
		strcat((char*)tmp,(const char*)w);
		strcat((char*)tmp,(const char*)str);

		if(appartient_au_dico(tmp))
			strcat((char*)w,(const char*)str);
		else{
			memcpy(*(dico + (indice_fin_dico++)),tmp, strlen((const char*)tmp)+1);
			donner_l_indice_de_w(fichier_dico,w);
			memcpy(w,str,2);
		}
		tmp[0]= '\0';
	}
	donner_l_indice_de_w(fichier_dico,w);
	free(w);
	free(tmp);
}

/*Fonction qui lit le fichier binaire compressé par paquet de 16 bits, reconstruit le dictionnaire au fur et à mesure et écrit les chaînes de caractères décompressées dans le fichier_ecraser*/
void LZW_D(FILE * fichier,FILE* fichier_ecraser){
	uint8_t *w=NULL,*tmp =NULL,str[2];
	uint16_t nombre_courant = 0;
	
	str[1] = '\0';

	if((w = malloc(20*sizeof(uint8_t))) == NULL)
		exit(EXIT_FAILURE);
	if((tmp = malloc(20*sizeof(uint8_t))) == NULL)
		exit(EXIT_FAILURE);

	fread(&nombre_courant, 1, sizeof(nombre_courant), fichier);
	fprintf(fichier_ecraser,"%s",*(dico+nombre_courant));
	str[0] = nombre_courant;
	strcpy((char*)w,(const char*)str);
	
	while(fread(&nombre_courant, 1, sizeof(nombre_courant), fichier)){
		if(nombre_courant > 255 && nombre_courant < indice_fin_dico)
			strcpy((char*)tmp,(const char*)*(dico+nombre_courant));
		else
			if(nombre_courant > 255 && nombre_courant > indice_fin_dico-1){
				str[0] = w[0];
				strcpy((char*)tmp,(const char*)w);
				strcat((char*)tmp,(const char*)str);
			}
			else{
				str[0] = nombre_courant;
				strcpy((char*)tmp,(const char*)str);
			}		
		fprintf(fichier_ecraser,"%s",tmp);
		str[0] = tmp[0];
		strcat((char*)w,(const char*)str);
		memcpy(*(dico + (indice_fin_dico++)),w,strlen((const char*)w)+1);
		strcpy((char*)w,(const char*)tmp);
	}
	free(w);
	free(tmp);
}

int main (int argc, char * argv[]){
	uint16_t i;
	uint8_t str[2],compresse = 0,decompresse = 0;
	FILE *fichier = NULL,*fichier_dico = NULL;
	
	/*Vérifie que la syntaxe donnée est correcte*/
	if(argc < 3)
		exit(EXIT_FAILURE);
	if(!strcmp(argv[1],"-c"))
		compresse = 1;
	else
		if(!strcmp(argv[1],"-d"))
			decompresse = 1;
	else{
		printf("LZW -d (decompresser) || -c (compresser) fichier_a_compresser ||fichier_a_ecraser");
		exit(EXIT_FAILURE);
	}

	/*Ouverture des fichiers en lecture/écriture binaire*/
	if(compresse){	
		fichier = fopen (argv[2], "r+");
		fichier_dico = fopen ("fichier_compresse.bin", "wb+");
	}
	if(decompresse){
		fichier = fopen (argv[2], "w+");
		fichier_dico = fopen ("fichier_compresse.bin", "rb+");
	}

	/*On quitte si on n'a pas pu ouvrir les fichiers*/
	if(fichier == NULL || fichier_dico == NULL)
		exit(EXIT_FAILURE);

	/*Allocation mémoire pour le dictionnaire*/
	if((dico = malloc(_TAILLE_MAX_DICO*sizeof(uint8_t*))) == NULL)
		exit(1);
	for(i=0;i<_TAILLE_MAX_DICO;i++)
		if((dico[i] = malloc(20*sizeof(uint8_t))) == NULL)
			exit(1);

	str[1] = '\0';

	/*Création du dictionnaire de base ASCII*/
	for(i=0;i<256;i++){
		str[0] = i;
		memcpy(*(dico + i),str,strlen((const char*)str)+1);
	}

	/*Compréssion ou décompression selon -c || -d */
	if (compresse)
		LZW(fichier,fichier_dico);
	if(decompresse)
		LZW_D(fichier_dico,fichier);

	/*Fermeture des fichiers*/
	fclose(fichier);
	fclose(fichier_dico);

	/*Libération du dictionnaire*/
	for(i=0;i<_TAILLE_MAX_DICO;i++)
		free(dico[i]);
	free(dico);
	
	return 0;
}
			

