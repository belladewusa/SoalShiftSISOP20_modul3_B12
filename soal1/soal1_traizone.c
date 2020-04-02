// gcc -pthread -o soal1_traizone soal1_traizone.c
#include <stdio.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <ctype.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

//Menu
#define FALSE 0
#define TRUE 1
#define CARI_POKEMON 2
#define POKEDEX 3
#define SHOP 4
#define BERHENTI_MENCARI 5
#define TANGKAP 6
#define ITEM 7
#define KELUAR_DARI_CAPTURE 8
#define LEPAS_POKEMON 9
#define BERI_BERRY 10

//Item
#define LULLABY_POWDER 0
#define POKEBALL 1
#define BERRY 2

// Shared memory index
#define LULLABY_STORE_NUM 0
#define POKEBALL_STORE_NUM 1
#define BERRY_STORE_NUM 2
#define INDEX_X_POKEMON 3
#define INDEX_Y_POKEMON 4
#define NEW_ESCAPE_RATE 5
#define NEW_CAPTURE_RATE 6
#define NEW_POKEDOLLAR 7
#define OLD_ESCAPE_RATE 8
#define OLD_CAPTURE_RATE 9
#define OLD_POKEDOLLAR 10
#define IS_LULLABY_USED 11

int is_capture_mode, is_cari_pokemon, pokemon_owned[7], count_pokemon_owned, pokedollar_owned, pokedollar_pokemon_owned[7], is_first_encounter;
int *shared_array;

char pokemon[3][5][100] = {
    {
        "Bulbasaur",
        "Charmander",
        "Squirtle",
        "Rattata",
        "Caterpie"
    },
    {
        "Pikachu",
        "Eevee",
        "Jigglypuff",
        "Snorlax",
        "Dragonite"
    },
    {
        "Mew",
        "Mewtwo",
        "Moltres",
        "Zapdos",
        "Articuno"
    }
};

char item[3][50] = {
    "Lullaby Powder",
    "Pokeball",
    "Berry"
};

int affection_point[7];
int index_pokemon[7][2];
int item_owned[3];
int encountered_pokemon[2];
int item_cost[] = {60, 5, 15};

void testingSharedArray() {
    printf("LULLABY_STORE_NUM %d\n", shared_array[LULLABY_STORE_NUM]);
    printf("POKEBALL_STORE_NUM %d\n", shared_array[POKEBALL_STORE_NUM]);
    printf("BERRY_STORE_NUM %d\n", shared_array[BERRY_STORE_NUM]);
    printf("INDEX_X_POKEMON %d\n", shared_array[INDEX_X_POKEMON]);
    printf("INDEX_Y_POKEMON %d\n", shared_array[INDEX_Y_POKEMON]);
    printf("NEW_ESCAPE_RATE %d\n", shared_array[NEW_ESCAPE_RATE]);
    printf("NEW_CAPTURE_RATE %d\n", shared_array[NEW_CAPTURE_RATE]);
    printf("NEW_POKEDOLLAR %d\n", shared_array[NEW_POKEDOLLAR]);
    printf("OLD_ESCAPE_RATE %d\n", shared_array[OLD_ESCAPE_RATE]);
    printf("OLD_CAPTURE_RATE %d\n", shared_array[OLD_CAPTURE_RATE]);
    printf("OLD_POKEDOLLAR %d\n", shared_array[OLD_POKEDOLLAR]);
    printf("IS_LULLABY_USED %d\n", shared_array[IS_LULLABY_USED]);
}

int checkInput(char *str) {
    int length = strlen(str), i;

    for ( i = 0; i < length; i++)
        str[i] = tolower(str[i]);
    
    if(!strcmp("cari pokemon", str)) return CARI_POKEMON;
    else if(!strcmp("berhenti mencari", str)) return BERHENTI_MENCARI;
    else if(!strcmp("pokedex", str)) return POKEDEX;
    else if(!strcmp("shop", str)) return SHOP;
    else if(!strcmp("tangkap", str)) return TANGKAP;
    else if(!strcmp("item", str)) return ITEM;
    else if(!strcmp("keluar dari capture", str)) return KELUAR_DARI_CAPTURE;
    else if(!strcmp("lepas pokemon", str)) return LEPAS_POKEMON;
    else if(!strcmp("beri berry", str)) return BERI_BERRY;

    return FALSE;
}

void *cariPokemon(void * ptr) {
    int chance_got;

    while (TRUE)
    {
        sleep(10);
        
        if(!is_cari_pokemon) break;

        chance_got = rand() % 100;

        if(!is_cari_pokemon) break;

        if(chance_got < 60) {
            is_cari_pokemon = FALSE;
            is_capture_mode = TRUE;
            is_first_encounter = TRUE;
            
            puts("\nMenemukan Pokemon!!");
            
            encountered_pokemon[0] = shared_array[INDEX_X_POKEMON];
            encountered_pokemon[1] = shared_array[INDEX_Y_POKEMON];

            shared_array[OLD_ESCAPE_RATE] = shared_array[NEW_ESCAPE_RATE];
            shared_array[OLD_POKEDOLLAR] = shared_array[NEW_POKEDOLLAR];
            shared_array[OLD_CAPTURE_RATE] = shared_array[NEW_CAPTURE_RATE];

            return NULL;
        } 
    }
}

void * pokemonRun(void *ptr) {
    while (TRUE)
    {
        sleep(20);
        
        if(!is_capture_mode) return NULL;

        int random_number = rand() % 100;

        if(random_number < shared_array[OLD_ESCAPE_RATE]) {
            puts("\nPokemon melarikan diri");
            
            is_capture_mode = FALSE;

            return NULL;
        }
    }
}

void * pokemonThread(void * ptr) {
    int *param = (int *)ptr;
    int index = param[0];

    while (TRUE)
    {
        sleep(10);
        // sleep(1000);

        if(!pokemon_owned[index]) return NULL;

        if(!is_capture_mode) affection_point[index] -= 10;
        // printf("AF -> %d", affection_point[index]);

        if(affection_point[index] <= 0) {
            int random_number = rand() % 100;

            if(random_number < 90) {
                printf("\nPokemon %s dengan index %d terlepas\n", pokemon[index_pokemon[index][0]][index_pokemon[index][1]], index);

                pokemon_owned[index] = FALSE;
                count_pokemon_owned--;
                return NULL;
            }else affection_point[index] = 50;
        }
    }
    return NULL;
}

int main() {
    int input_num, i;
    char input[1024];
    is_capture_mode = is_cari_pokemon =  FALSE;
    count_pokemon_owned = pokedollar_owned = 0;

    for (i = 0; i < 7; i++)
        pokemon_owned[i] = affection_point[i] = FALSE;
    
    for ( i = 0; i < 3; i++)
        item_owned[i] = 0;

    item_owned[POKEBALL] = 30;
    
    pthread_t thread_pokemons[7];
    pthread_t thread_pencarian;
    pthread_t pokemon_run;

    key_t key = 1234;

    int shmid = shmget(key, sizeof(int)*20, IPC_CREAT | 0666);
    shared_array = shmat(shmid, NULL, 0);

    while (TRUE)
    {        
        printf("\nPokedollar dimiliki : %d\nPokemon dimiliki : %d\nItem dimiliki : %d\n", pokedollar_owned, count_pokemon_owned, item_owned[BERRY] + item_owned[LULLABY_POWDER] + item_owned[POKEBALL]);
        if(!is_capture_mode) {
            puts("Menu :");
            if(!is_cari_pokemon)puts("1.Cari Pokemon");
            else puts("1.Berhenti Mencari");
            puts("2.Pokedex\n3.Shop");
            if(is_capture_mode) puts("Pokemon Ditemukan, masukan input apa saja untuk meneruskan ke capture mode");
            printf("Input (tuliskan menu) : ");

            gets(input);
            if(is_capture_mode) continue;
            input_num = checkInput(input);

            switch (input_num)
            {
            case CARI_POKEMON:
                putchar('\n');
                if(is_cari_pokemon) {
                    puts("Sedang dalam mode mencari");
                    continue;
                }

                is_first_encounter = FALSE;

                int iret_pencarian;

                is_cari_pokemon = TRUE;

                iret_pencarian = pthread_create(&thread_pencarian, NULL, cariPokemon, NULL);
                if(iret_pencarian) {
                    fprintf(stderr,"Error - pthread_create() return code: %d\n",iret_pencarian);
                    exit(EXIT_FAILURE);
                }

                puts("Jika muncul pesan \"Menemukan Pokemon!!\" silahkan berikan input seperti perintah yang sebelumnya, setelahnya anda akan berpindah ke capture mode");

                break;
            case BERHENTI_MENCARI:
                putchar('\n');
                if(!is_cari_pokemon) {
                    puts("Tidak sedang dalam mode mencari");
                    continue;
                }

                is_cari_pokemon = FALSE;

                break;
            case POKEDEX:
                putchar('\n');
                if(count_pokemon_owned < 1) {
                    puts("Tidak ada pokemon yang dimiliki");
                    continue;
                }

                for ( i = 0; i < 7; i++)
                {
                    if(pokemon_owned[i]) {
                        printf("%d.Pokemon : %s | AP : %d\n", i, pokemon[index_pokemon[i][0]][index_pokemon[i][1]], affection_point[i]);
                    }
                }

                puts("Menu :\n1.Lepas Pokemon\n2.Beri Berry\n(Untuk keluar beri masukan lain)");
                printf("Input : ");

                gets(input);
                if(is_capture_mode) continue;

                input_num = checkInput(input);

                switch (input_num)
                {
                case LEPAS_POKEMON:
                    puts("Masukkan index dari pokemon yang ingin dilepas \n(Masukkan index selain angka itu untuk keluar)");
                    printf("Index : ");
                    
                    int num;
                    scanf("%d", &num);
                    getchar();
                    if(is_capture_mode) continue;

                    if(num > 6 || num < 0 || !pokemon_owned[num]){
                        puts("Input salah");
                        continue;
                    }

                    pokedollar_owned += pokedollar_pokemon_owned[num];

                    pokemon_owned[num] = FALSE;

                    count_pokemon_owned--;

                    break;
                case BERI_BERRY:
                    putchar('\n');
                    if(item_owned[BERRY] < 1) {
                        puts("Anda Tidak Memiliki Berry");
                        continue;
                    }

                    item_owned[BERRY] -= 1;

                    for ( i = 0; i < 7; i++)
                    {
                        if(pokemon_owned[i])
                            affection_point[i] += 10;
                    }
                    
                    break;
                default:
                    if(!is_capture_mode) puts("Input salah");
                    break;
                }
                
                break;
            case SHOP:
                putchar('\n');
                puts("Item yang kamu miliki :");
                for ( i = 0; i < 3; i++)
                    printf("%s Jumlah : %d\n", item[i], item_owned[i]);
                putchar('\n');
                puts("Shop :");
                for ( i = 0; i < 3; i++)
                {
                    printf("%d.%s Stok : %d Harga : %d/item\n", i, item[i], shared_array[i], item_cost[i]);
                }
                
                printf("Masukkan index item dan jumlah (Contoh: 2 10 (Membeli 10 Berry)) : ");
                
                int index_item, jumlah;
                scanf("%d %d", &index_item, &jumlah);
                getchar();
                if(is_capture_mode) continue;

                if(index_item < 0 || index_item > 2 || shared_array[index_item] < jumlah || (item_owned[index_item] + jumlah > 99) || ((jumlah * item_cost[index_item]) > pokedollar_owned ) || jumlah < 1){
                    puts("Input salah atau item di store kurang atau jumlah item yang akan dimiliki terlalu banyak atau Pokedollar anda kurang");
                    continue;
                }

                item_owned[index_item] += jumlah;
                shared_array[index_item] -= jumlah;
                pokedollar_owned -= (jumlah * item_cost[index_item]);

                // testingSharedArray();
                break;
            default:
                if(!is_capture_mode) puts("Input salah");
                break;
            }
        //Capture Mode
        }else{
            if(is_first_encounter) {

                int iret = pthread_create(&pokemon_run, NULL, pokemonRun, NULL);

                if(iret) {
                    fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
                    exit(EXIT_FAILURE);
                }

                is_first_encounter = FALSE;
            }

            printf("Escape Rate : %d/100 \nCapture Rate : %d/100 ", shared_array[OLD_ESCAPE_RATE], shared_array[OLD_CAPTURE_RATE]);
            puts("\nMenu :");
            puts("1.Tangkap\n2.Item\n3.Keluar dari capture");
            printf("Input (tuliskan menu) : ");

            gets(input);
            if(!is_capture_mode) continue;
            input_num = checkInput(input);

            switch (input_num)
            {
            case TANGKAP:
                putchar('\n');
                if(item_owned[POKEBALL] < 1) {
                    puts("Pokeball tidak cukup");
                    continue;
                }

                item_owned[POKEBALL] -= 1;

                int random_number = rand() % 100;
                
                if(random_number < shared_array[OLD_CAPTURE_RATE] && is_capture_mode) {
                    is_capture_mode = FALSE;
                    puts("Berhasil meangkap pokemon");
                    
                    if(count_pokemon_owned >= 7) {
                        puts("Karena slot pokemon penuh, pokedollar ditambahkan");

                        pokedollar_owned += shared_array[OLD_POKEDOLLAR];
                        continue;
                    }

                    for ( i = 0; i < 7; i++)
                    {
                        if(!pokemon_owned[i]) {
                            pokemon_owned[i] = TRUE;

                            affection_point[i] = 100;
                            
                            index_pokemon[i][0] = encountered_pokemon[0];
                            index_pokemon[i][1] = encountered_pokemon[1];

                            pokedollar_pokemon_owned[i] = shared_array[OLD_POKEDOLLAR];

                            count_pokemon_owned++;

                            int arr[1] = {i};
                        
                            int iret = pthread_create(&thread_pokemons[i], NULL, pokemonThread, (void *) arr);
                            
                            if(iret) {
                                fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
                                exit(EXIT_FAILURE);
                            }
                            break;
                        }
                    }
                }else puts("Gagal menangkap");

                break;
            case ITEM:
                putchar('\n');
                if(item_owned[LULLABY_POWDER] < 1) {
                    puts("Anda tidak memiliki lullaby powder saat ini");
                    continue;
                }

                item_owned[LULLABY_POWDER] -= 1;

                shared_array[IS_LULLABY_USED] = TRUE;

                puts("\nBerhasil menggunakan Lullaby Powder, apabila belum kelihatan efeknya silahkan refresh");

                break;
            case KELUAR_DARI_CAPTURE:
                is_capture_mode = FALSE;
                break;
            default:
                if(is_capture_mode) puts("Input salah");
                break;
            }
        }
    }

    shmdt(shared_array);
    shmctl(shmid, IPC_RMID, NULL);
}