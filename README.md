# SoalShiftSISOP20_modul3_B12

## Soal 1

Jadi soal ini disuruh untuk membuat 2 kodingan yaitu `soal1_traizone.c` dan `soal1_pokezone.c`. `soal1_traizone.c` digunakan untuk trainernya dimana memiliki dua mode yaitu Normal Mode dan Captura Mode. Di Normal Mode ada 3 menu yaitu Cari Pokemon, Pokedex, dan Shop. Untuk Capture Mode ada 3 menu yaitu Tangkap, Item, dan Keluar. Lalu untuk file `soal1_pokezone.c` memiliki beberapa tugas diantaranya shutdown untuk program `soal1_traizone` sekaligus `soal1_pokezone`, lalu menyediakan pokemon yan dirandom untuk `soal1_traizone`, lalu menyediakan item untuk dibeli `soal1_traizone`.

Di file `soal1_traizone.c` menggunakan beberapa library dan variabel global diantaranya :
```bash
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
```
Lalu di file `soal1_traizone.c` ada beberapa fungsi diantaranya :
- `checkInput` untuk mengecek inputan menu dari user.
```bash
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
    else if(!strcmp("keluar", str)) return KELUAR_DARI_CAPTURE;
    else if(!strcmp("lepas pokemon", str)) return LEPAS_POKEMON;
    else if(!strcmp("beri berry", str)) return BERI_BERRY;

    return FALSE;
}
```
- `cariPokemon` untuk thread menu Cari Pokemon 
```bash
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
```

- `pokemonRun`untuk thread kemungkinan pokemon kabur saat Capture Mode
```bash
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
```

- `pokemonThread` fungsi untuk thread affection dari pokemon
```bash
void * pokemonThread(void * ptr) {
    int *param = (int *)ptr;
    int index = param[0];

    while (TRUE)
    {
        sleep(10);

        if(!pokemon_owned[index]) return NULL;

        if(!is_capture_mode) affection_point[index] -= 10;

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
```

- Lalu main function berisi menu dan sedikit logika dengan shared memory :
```bash
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
            puts("1.Tangkap\n2.Item\n3.Keluar\nUntuk ketik selain perintah di menu");
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

                if(!is_capture_mode) continue;
                
                if(item_owned[LULLABY_POWDER] < 1) {
                    puts("Anda tidak memiliki lullaby powder saat ini");
                    continue;
                }

                if(shared_array[OLD_CAPTURE_RATE] >= 100) {
                    puts("Capture Rate sudah full");
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
```

Untuk `soal1_pokezone` memiliki beberapa header dan variabel global :
```bash
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
#include <fcntl.h>
#include <sys/stat.h>
#include <wait.h>
#include <time.h>
#include <sys/prctl.h>

//General
#define FALSE 0
#define TRUE 1

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

int escape_rate[] = {5, 10, 20};
int capture_rate[] = {70, 50, 30};
int pokedollar[] = {80, 100, 200};
int *shared_array;
int lullaby_check[100];
int count_lullaby_effect_running;
int first_escape_rate;
```

Lalu disini ada beberapa fungsi diantaranya :
- `itemAdding` untuk thread dimana per 10 detik semua item bertambah 10
```bash
void * itemAdding(void * ptr) {
    int i;
    while (TRUE)
    {
        for ( i = 0; i < 30; i++)
        {
            if((shared_array[LULLABY_STORE_NUM] + shared_array[BERRY_STORE_NUM] + shared_array[POKEBALL_STORE_NUM]) >= 200) break;

            shared_array[i % 3] += 1;
        }

        sleep(10);
    }
    
    return NULL;
}
```

- `lullabyEffect` thread saat trainer menggunakan item Lullaby Powder
```bash
void * lullabyEffect(void * ptr) {
    int *param = (int *)ptr;

    int i, count = 0;

    if(count_lullaby_effect_running <= 0)
        first_escape_rate = shared_array[OLD_ESCAPE_RATE];
    
    count_lullaby_effect_running++;

    for ( i = 0; i < 20; i++)
    {
        if(shared_array[OLD_CAPTURE_RATE] >= 100) break;
        shared_array[OLD_CAPTURE_RATE] += 1;
        count++;
    }

    shared_array[OLD_ESCAPE_RATE] = 0;
    
    sleep(10);
        
    shared_array[OLD_CAPTURE_RATE] -= count;
    
    if(count_lullaby_effect_running == 1)
        shared_array[OLD_ESCAPE_RATE] = first_escape_rate;

    count_lullaby_effect_running--;

    lullaby_check[param[0]] = FALSE;

    return NULL;
}
```

- `randomPokemon` fungsi untuk thread random pokemon per detik
```bash
void * randomPokemon( void * ptr) {
    while (TRUE)
    {
        int random_num = rand() % 100;
        if(random_num < 80) shared_array[INDEX_X_POKEMON] = 0;
        else if(random_num < 95) shared_array[INDEX_X_POKEMON] = 1;
        else shared_array[INDEX_X_POKEMON] = 2;

        random_num = rand() % 5;

        shared_array[INDEX_Y_POKEMON] = random_num;

        shared_array[NEW_ESCAPE_RATE] = escape_rate[shared_array[INDEX_X_POKEMON]];
        shared_array[NEW_CAPTURE_RATE] = capture_rate[shared_array[INDEX_X_POKEMON]];
        shared_array[NEW_POKEDOLLAR] = pokedollar[shared_array[INDEX_X_POKEMON]];

        random_num = rand() % 8000;
        if(random_num == 77) {
            shared_array[NEW_ESCAPE_RATE] += 5;
            shared_array[NEW_CAPTURE_RATE] -= 20;
            shared_array[NEW_POKEDOLLAR] += 5000;
        }

        sleep(1);
    }
    return NULL;
}
```

- Lalu main function
```bash
int main() {

    key_t key = 1234;
    int shmid = shmget(key, sizeof(int)*20, IPC_CREAT | 0666), i;
    shared_array = shmat(shmid, NULL, 0);
    int irett[2];
    count_lullaby_effect_running = 0;

    pthread_t item_adding;
    pthread_t lullaby_effect[100];
    pthread_t random_pokemon;

    for ( i = 0; i < 12; i++) shared_array[i] = 0;
    shared_array[POKEBALL_STORE_NUM] = 40;
    shared_array[LULLABY_STORE_NUM] = shared_array[BERRY_STORE_NUM] = 30;
    shared_array[IS_LULLABY_USED] = FALSE;

    irett[0] = pthread_create(&item_adding, NULL, itemAdding, NULL);
    irett[1] = pthread_create(&random_pokemon, NULL, randomPokemon, NULL);

    for ( i = 0; i < 100; i++) lullaby_check[i] = FALSE;
    
    pid_t checking_lullaby = fork();

    if(checking_lullaby == 0) {
        while (TRUE)
        {
            if(shared_array[IS_LULLABY_USED]) {
                for ( i = 0; i < 100; i++)
                {
                    if(!lullaby_check[i]) {
                        lullaby_check[i] = TRUE;

                        int iret;
                        int arr[1] = {i};

                        iret = pthread_create(&lullaby_effect[i], NULL, lullabyEffect, (void*) arr);
                        break;
                    }
                }
                
                shared_array[IS_LULLABY_USED] = FALSE;
            }
        }
    }

    char message[100];

    while (TRUE)
    {
        puts("Ketik shutdown untuk shutdown game");
        
        gets(message);

        int length = strlen(message);
        
        for ( i = 0; i < length; i++)
        {
            message[i] = tolower(message[i]);
        }

        if(!strcmp("shutdown", message)) break;
        puts("Input salah");   
    }

    pid_t shutdown = fork();

    if(shutdown == 0) {
        char *arr[] = {"pkill","-f","soal1_traizone",NULL};
        execv("/bin/pkill",arr);
    }

    shmdt(shared_array);
    shmctl(shmid, IPC_RMID, NULL);
    exit(EXIT_SUCCESS);
}
```

Kesulitan :
- Tidak ada kesulitan berarti selain kerumitannya

## Soal 2

Lalu di soal 2 ini kita disuruh untuk membuat suatu game dengan nama TapTap Game dimana dia terdiri dari 2 file yaitu `tapplayer.c` dan `tapserver.c`. Aplikasi ini menggunakan socket untuk melakukan multi client nya dan berkomunikasi antara client dengan server. Di `tapserver.c` program menyediakan file `akun.txt` dipakai menyimpan username dan password, mengautentikasi login dari user, dan menyambungkan dengan user lain saat seorang user ingin bermain. Lalu di `tapplayer.c` ada login, register, menemukan lawan main, dan logout.

Di file `tapserver.c` menggunakan beberapa library dan variabel global yaitu :
```bash
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <pthread.h>
#include <sys/wait.h> 
#include <sys/types.h> 

#define PORT 8080 
#define MAX_CLIENT 30
#define FALSE 0
#define TRUE 1
#define LOGIN 2
#define REGISTER 3
#define FIND_MATCH 4
#define LOGOUT 5
#define WIN 6
#define LOSE 7

int searching;
int health[100];

void *client_thread( void *ptr );
```

Lalu fungsi `client_thread` sendiri mengatur thread untuk 1 client, fungsinya :
```bash
void *client_thread(void *ptr) {
	int valread;
	char buffer[1024], message[1024], username[1024], password[1024];
	int *new_socket = (int *)ptr;
	int client_addr = new_socket[0];
    FILE *fptr;
	
    memset(buffer, 0, sizeof(buffer));
    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));
    memset(message, 0, sizeof(message));

	while ((valread = recv(client_addr , buffer , 1024 , 0)) > 0)
	{
        if(!strcmp("login", buffer)) {
            fptr = fopen("akun.txt","rb");
            int flag = FALSE;
            if(fptr != NULL) {
                memset(buffer, FALSE, sizeof(buffer));

                valread = recv(client_addr , username , 1024 , 0);
                valread = recv(client_addr , password , 1024 , 0);

                char temp[2][1024];

                while(fread(temp, sizeof(temp), 2, fptr))
                {
                    if(!strcmp(temp[0], username) && !strcmp(temp[1], password)) {
                        flag = TRUE;
                        send(client_addr , "login success" , strlen("login success") , 0 );
                        printf("Auth success\n");
                        break;
                    }
                }

                fclose(fptr);
            }

            if(!flag){
                send(client_addr , "login failed" , strlen("login failed") , 0 );
                printf("Auth Failed\n");
            }

        }else if(!strcmp("register", buffer)) {
            fptr = fopen("akun.txt","ab");
            char temp[2][1024];
            
            memset(buffer, FALSE, sizeof(buffer));

            memset(temp[0], FALSE, sizeof(temp[0]));
            memset(temp[1], FALSE, sizeof(temp[1]));
            
            valread = recv(client_addr , temp[0] , 1024 , 0);

            valread = recv(client_addr , temp[1] , 1024 , 0);

            send(client_addr , "register success" , strlen("register success") , 0 );

            fwrite(temp , sizeof(temp) , 2 ,  fptr );

            fclose(fptr);

            fptr = fopen("akun.txt","rb");
            if(fptr != NULL) {
                while (fread(temp, sizeof(temp), 2, fptr))
                {
                    printf("Username : %s Password : %s\n", temp[0], temp[1]);
                }
                
                fclose(fptr);
            }
        }else if(!strcmp("waiting for player", buffer)) {
            int opponent_addr;
            if(searching == -1){
                searching = client_addr;

                while (searching == client_addr)
                {
                }

                opponent_addr = searching;

            } else {
                opponent_addr = searching;
                searching = client_addr;

                sleep(1);
                searching = -1;
                send(client_addr, "found your opponent", strlen("found your opponent"), 0);
                send(opponent_addr, "found your opponent", strlen("found your opponent"), 0);
                health[client_addr] = health[opponent_addr] = 100;
            }

            int flag = WIN;

            while ((valread = recv(client_addr , buffer , 1024 , 0)) > 0)
            {
                health[opponent_addr] -= 10;

                if(health[opponent_addr] <= 0) {
                    send(client_addr, "win", strlen("win"), 0);
                    send(opponent_addr, "lose", strlen("lose"), 0);
                    break;
                }

                if(health[client_addr] <= 0) {
                    // send(client_addr, "lose", strlen("lose"), 0);
                    // send(opponent_addr, "win", strlen("win"), 0);
                    break;
                }

                send(client_addr, "hit !!", strlen("hit !!"), 0);
                
                memset(message, FALSE, sizeof(message));
                sprintf(message, "Health kamu sekarang : %d", health[opponent_addr]);
                send(opponent_addr, message, sizeof(message), 0);
                memset(buffer, 0, sizeof(buffer));
            }
            memset(buffer, 0, sizeof(buffer));
        }
		
        memset(buffer, 0, sizeof(buffer));
        memset(username, 0, sizeof(username));
        memset(password, 0, sizeof(password));
        memset(message, 0, sizeof(message));
	}
}
```

Lalu main functionnya :
```bash
int main(int argc, char const *argv[]) 
{ 
	int server_fd, new_socket
	[MAX_CLIENT]
	, valread, i, iret[MAX_CLIENT]; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address);  
	pthread_t threads[MAX_CLIENT];
    FILE *fptr;
    fptr = fopen("akun.txt","ab");
    fclose(fptr);

    searching = -1;
    for ( i = 0; i < 100; i++)
        health[i] = 100;
	
	// Creating socket file descriptor 
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	// Forcefully attaching socket to the port 8080 
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR , &opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 

	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 
	
	// Forcefully attaching socket to the port 8080 
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	}

	if (listen(server_fd, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 

	for ( i = 0; i < MAX_CLIENT; i++)
	{
		if ((new_socket[i] = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
		{ 
			perror("accept"); 
			exit(EXIT_FAILURE); 
		}

		void *ress;
		int arr[1] = {new_socket[i]};

		iret[i] = pthread_create(&threads[i], NULL, client_thread, (void*) arr);
                
		if(iret[i]) //jika eror
		{
			fprintf(stderr,"Error - pthread_create() return code: %d\n",iret[i]);
			exit(EXIT_FAILURE);
		}
	}

	for ( i = 0; i < MAX_CLIENT; i++)
	{
		pthread_join(threads[i], NULL);
	}
	
	return 0;
} 
```

Lalu untuk file `tapplayer.c` memiliki header, beberapa constant, dan variabel global
```bash
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <ctype.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <pthread.h>
#include <stdlib.h>
#include <termios.h>

#define PORT 8080 
#define FALSE 0
#define TRUE 1
#define LOGIN 2
#define REGISTER 3
#define FIND_MATCH 4
#define LOGOUT 5
#define UNAUTHENTICATED 6
#define AUTHENTICATED 7
#define BATTLE 8
#define WIN 9
#define LOSE 10

int status, sock, valread;

static struct termios stored_settings;
```

Ada beberapa fungsi di `tapplayer.c` diantaranya :
- `set_keypress` untuk setting agar bisa input tanpa enter
```bash
void set_keypress(void)
{
    struct termios new_settings;
    tcgetattr(0,&stored_settings);
    new_settings = stored_settings;
    new_settings.c_lflag &= (~ICANON);
    new_settings.c_cc[VTIME] = 0;
    new_settings.c_cc[VMIN] = 1;
    tcsetattr(0,TCSANOW,&new_settings);
    return;
}
```

- `reset_keypress` mengembalikan state yang diciptakan `set_keypress`
```bash
void reset_keypress(void)
{
    tcsetattr(0,TCSANOW,&stored_settings);
    return;
}
```

- `checkInput` untuk cek input apa yang dimasukkan saat dalam menu
```bash
int checkInput(char *input) {
    int length = strlen(input), i;
    
    for ( i = 0; i < length; i++)
    {
        input[i] = tolower(input[i]);
    }
    
    if(!strcmp(input, "login")) return LOGIN;
    else if(!strcmp(input, "register")) return REGISTER;
    else if(!strcmp(input, "find match") || !strcmp(input, "find")) return FIND_MATCH;
    else if(!strcmp(input, "logout")) return LOGOUT;
    return FALSE;
}
```

- `loop` untuk thread looping print 'Waiting for player...'
```bash
void *loop( void *ptr )
{
    while (status == TRUE)
    {
        if(status == BATTLE || status == FALSE)break;
        puts("Waiting for player ...");
        sleep(1);
    } 
    return NULL;
}
```

- `receiveInput` fungsi untuk thread input dari server saat dalam state battle
```bash
void *receiveInput( void *ptr )
{
    char buffer[1024];
    memset(buffer, FALSE, sizeof(buffer));

    while((valread = read(sock, buffer, 1024))) {
        if(!strcmp(buffer, "win")) {
            printf("Game berakhir kamu menang\n");
            memset(buffer, FALSE, sizeof(buffer));
            status = WIN;
            return NULL;
        }

        if(!strcmp(buffer, "lose")) {
            printf("Game berakhir kamu kalah\n");
            memset(buffer, FALSE, sizeof(buffer));
            status = LOSE;
            return NULL;
        }

        printf("%s\n", buffer);

        memset(buffer, FALSE, sizeof(buffer));
    }

    status = FALSE;
    return NULL;
}
```

- Lalu main function
```bash
int main(int argc, char const *argv[]) 
{ 
	int is_login = 0, flag = 0, iret; 
	struct sockaddr_in serv_addr; 
	char message[1024], buffer[1024] = {0}, c;
    pthread_t thread;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		printf("\n Socket creation error \n"); 
		return -1; 
	} 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(PORT); 
	
	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
	{ 
		printf("\nInvalid address/ Address not supported \n"); 
		return -1; 
	} 

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		printf("\nConnection Failed \n"); 
		return -1; 
	}

    while (1)
    {
        memset(buffer, FALSE, sizeof(buffer));
        memset(message, FALSE, sizeof(message));

        if(flag != FALSE) {
            if(flag == AUTHENTICATED){
                printf("Sudah Login\n");
            }else if(flag == LOGIN) {
                printf("Login Terlebih Dahulu\n");
            }
            flag = FALSE;
        }

        if(!is_login)printf("\n1.Login\n2.Register\nChoices : ");
        else printf("\n1.Find Match\n2.Logout\nChoices : ");
        
        gets(message);
        int input = checkInput(message);
        memset(message, FALSE, sizeof(message));

        switch (input)
        {
        case LOGIN:
            if(is_login){
                flag = AUTHENTICATED;
                printf("Input Salah\n");
                continue;
            }
            
            send(sock , "login" , strlen("login") , 0 ); 
            
            printf("Username : ");
            gets(message);
            
            if(strlen(message) < 1){
                puts("Input salah atau harus lebih dari 1 karakter");
                continue;
            }

            send(sock , message , strlen(message) , 0 );
            memset(message, FALSE, sizeof(message));

            printf("Password : ");
            gets(message);
            
            if(strlen(message) < 1){
                puts("Input salah atau harus lebih dari 1 karakter");
                continue;
            }
            
            send(sock , message , strlen(message) , 0 );
            memset(message, FALSE, sizeof(message));
            
            valread = read(sock, buffer, 1024);
            puts(buffer);

            if(!strcmp("login success", buffer)) is_login = TRUE;

            break;
        case REGISTER:
            if(is_login){
                flag = AUTHENTICATED;
                printf("Input Salah\n");
                continue;
            }

            send(sock , "register" , strlen("register") , 0 );
            memset(message, FALSE, sizeof(message)); 
            
            printf("Username : ");
            gets(message);
            
            if(strlen(message) < 1){
                puts("Input salah atau harus lebih dari 1 karakter");
                continue;
            }

            send(sock , message , strlen(message) , 0 );
            memset(message, FALSE, sizeof(message));

            printf("Password : ");
            gets(message);

            if(strlen(message) < 1){
                puts("Input salah atau harus lebih dari 1 karakter");
                continue;
            }

            send(sock , message , strlen(message) , 0 );
            memset(message, FALSE, sizeof(message));

            valread = read(sock, buffer, 1024);
            puts(buffer);

            break;
        case FIND_MATCH:
            if(!is_login){
                flag = UNAUTHENTICATED;
                printf("Input Salah\n");
                continue;
            }

            status = TRUE;

            send(sock , "waiting for player" , strlen("waiting for player"), 0 );

            iret = pthread_create(&thread,NULL,&loop,NULL);

            if(iret)
            {
                fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
                exit(EXIT_FAILURE);
            }

            while (valread = read(sock, buffer, 1024))
            {
                if(!strcmp(buffer,"found your opponent")){
                    status = FALSE;
                    break;
                }

                memset(buffer, FALSE, sizeof(buffer));
            }

            printf("Game dimulai silahkan tap tap secepat mungkin !!\n");

            sleep(1);

            status = BATTLE;

            iret = pthread_create(&thread,NULL,&receiveInput,NULL);

            if(iret)
            {
                fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
                exit(EXIT_FAILURE);
            }

            set_keypress();

            while (status == BATTLE)
            {
                if(status != BATTLE) break;
                c = getc(stdin);
                if(status == WIN) break;
                if(c == 32) {
                    if(status != WIN) send(sock , "hit" , strlen("hit"), 0 );
                }
            }
            status = FALSE;

            reset_keypress();

            break;
        case LOGOUT:
            if(!is_login){
                flag = UNAUTHENTICATED;
                printf("Input Salah\n");
                continue;
            }
            
            is_login = FALSE;
            break;
        default:
            printf("Input Salah\n");
            break;
        }
    }
    
	return 0; 
} 
```

Kesulitan :
- Buffer lupa belum dibersihkan di sisi server
- Ada pesan yang terkirim secara dobel

## Soal 3

Lalu soal 3 ini kita disuruh untuk membuat suatu program yang bisa mengkategorisasi file sesuai dengan ektensi nya dalam 1 level saja (tidak perlu rekursif), lalu bisa dijalankan dalam 3 mode '-f' (deretan file yang dimasukkan di argumen program), '-d' (1 folder), atau '*' (file di directory program dijalankan).

Program ini memiliki beberapa header dan constant diantaranya :
```bash
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wait.h>
#include <time.h>
#include <stdbool.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <libgen.h>

#define TRUE 1
#define FALSE 0
#define F 2
#define D 3
#define STAR 4
```

Lalu di program ini ada beberapa fungsi :
- `isDirectory` cek apakah path yang dimasukkan adalah path ke directory atau file
```bash
int isDirectory(const char *path) {
   struct stat statbuf;
   if (stat(path, &statbuf) != 0)
       return FALSE;
   return S_ISDIR(statbuf.st_mode);
}
```

- `checkAllFileExist` cek apakah semua path file yang dimasukkan ada
```bash
int checkAllFileExist(int argc, char *argv[]) {
    int i;
    char curr_dirr[1000];
    getcwd(curr_dirr, sizeof(curr_dirr));
    strcat(curr_dirr, "/");
    for ( i = 2; i < argc; i++)
    {
        char temp[1000];
        if(argv[i][0] != '/') {
            strcpy(temp, curr_dirr);
            strcat(temp, argv[i]);
        }else strcpy(temp, argv[i]);
        struct stat buffer;
        if(stat(argv[i], &buffer) != 0 || isDirectory(temp)) return FALSE;
    }
    return TRUE;
}
```

- `get_filename_ext` untuk mendapat ekstensi dari suatu file
```bash
const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";

    return dot;
}
```

- `checkMainArg` checking argumen yang dimasukkan ke program
```bash
int checkMainArg(int argc, char *argv[]) {
    if(argc < 2)
        return FALSE;
    if(argv[1][0] == '*' && strlen(argv[1]) == 1 && argc == 2)
        return STAR;   
    if(argv[1][0] == '-' && strlen(argv[1]) == 2)
        switch (argv[1][1])
        {
        case 'f':
            return (argc < 3)? FALSE : (strlen(argv[1]) != 2)? FALSE : (!checkAllFileExist(argc, argv))? FALSE : F;
        case 'd':
            return (argc != 3)? FALSE : (strlen(argv[1]) != 2)? FALSE : (!isDirectory(argv[2]))? FALSE : D;
        }

    return FALSE;
}
```

- `moveFileToNewPath` memindahkan file ke path baru
```bash
void moveFileToNewPath(char *source, char *dest) {
    FILE *fp1, *fp2;
    int ch;
    
    fp1 = fopen(source, "r");
    fp2 = fopen(dest, "w");

    while ((ch = fgetc(fp1)) != EOF) fputc(ch, fp2);
    
    fclose(fp1);
    fclose(fp2);

    remove(source);
}
```

- `moveFile` fungsi untuk memindahkan ke suatu directory
```bash
void *moveFile( void *ptr )
{
    char *param = (char *)ptr;
    const char *extension_temp = get_filename_ext(param);
    int ch, i, length = strlen(extension_temp);
    char extension[100];
    char *filename = basename(param);
    char new_path[1000],temp[1000];

    memset(new_path, 0, sizeof(new_path));
    memset(extension, 0, sizeof(extension));

    for ( i = 1; i < length; i++)
    {
        extension[i - 1] = tolower(extension_temp[i]);
    }
    
    
    if(!strlen(extension_temp)) {
        if(!isDirectory("Unknown"))mkdir("Unknown", 0777);
        strcpy(new_path, "Unknown/");
        strcat(new_path, filename);
    }else{
        if(!isDirectory(extension))mkdir(extension, 0777);
        strcpy(new_path, extension);
        strcat(new_path, "/");
        strcat(new_path, filename);
    }

    moveFileToNewPath(param, new_path);
}
```

- Lalu di main function 
```bash
int main(int argc, char *argv[]) {
    int check = checkMainArg(argc, argv), i = 0, iret[10000], let = 1;

    if(!check) {
        printf("Argumen Salah!\n");
        exit(EXIT_FAILURE);
    }

    char curr_dirr[1000];
    pthread_t threads[10000];
    struct dirent *ep;     
    DIR *dp;

    switch (check)
    {
    case F:
        for (i = 2; i < argc; i++)
        {
            char *arr = argv[i];
            iret[i - 2] = pthread_create(&threads[i - 2], NULL, moveFile, (void*) arr);
            
            if(iret[i - 2])
            {
                fprintf(stderr,"Error - pthread_create() return code: %d\n",iret[i - 2]);
                exit(EXIT_FAILURE);
            }

            pthread_join(threads[i - 2], NULL); 
        }
        break;
    case D:
        getcwd(curr_dirr, sizeof(curr_dirr));
        if(argv[2][0] != '/'){
            strcat(curr_dirr, "/");
            strcat(curr_dirr, argv[2]);
            if(argv[2][strlen(argv[2]) - 1] != '/')
                strcat(curr_dirr, "/");
        }
        dp = opendir (argv[2]);
        if(dp != NULL) {
            while (ep = readdir (dp)){
                char temp[1000];
                strcpy(temp, curr_dirr);
                strcat(temp, ep->d_name);
                if(strcmp(".",ep->d_name) !=0 && strcmp("..",ep->d_name) != 0 && !isDirectory(temp)) {
                    strcpy(temp, argv[2]);
                    if(argv[2][strlen(argv[2]) - 1] != '/') strcat(temp, "/");
                    strcat(temp, ep->d_name);
                    char *arr = temp;
                    iret[i] = pthread_create(&threads[i], NULL, moveFile, (void*) arr);
            
                    if(iret[i])
                    {
                        fprintf(stderr,"Error - pthread_create() return code: %d\n",iret[i]);
                        exit(EXIT_FAILURE);
                    }

                    pthread_join(threads[i], NULL); 
                }
                i++;
            }
        }
        break;
    case STAR:
        if(getcwd(curr_dirr, sizeof(curr_dirr)) != NULL) {
            dp = opendir (curr_dirr);
            if(dp != NULL) {
                while (ep = readdir (dp)){
                    char temp[1000];
                    strcpy(temp, curr_dirr);
                    strcat(temp, "/");
                    strcat(temp, ep->d_name);
                    if(strcmp(".",ep->d_name) !=0 && strcmp("..",ep->d_name) != 0 && !isDirectory(temp)) {
                        char *arr = temp;
                        iret[i] = pthread_create(&threads[i], NULL, moveFile, (void*) arr);
                
                        if(iret[i])
                        {
                            fprintf(stderr,"Error - pthread_create() return code: %d\n",iret[i]);
                            exit(EXIT_FAILURE);
                        }

                        pthread_join(threads[i], NULL); 
                    }
                    i++;
                }
            }
        }
        break;
    }
}
```

Kesulitan :
- Untuk multi-threading nya masih salah
- Dan belum bisa di fix saat revisi

## Soal 4
Disini kita disuruh membuat 3 file yaitu `soal4a.c`, `soal4b.c`, dan `soal4c.c`. `soal4a.c` mengalikan matriks lalu ditampilkan, `soal4b.c` menampilkan hasil perkalian dari `soal4a.c` dan melakukan penambahan setiap angka dari 1 sampai angka itu, `soal4c.c` melakukan pipe dengan IPC-pipes command `ls | wc -l`.

Untuk `soal4a.c` memiliki header :
```bash
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define X1 4
#define X2 5
#define Y 2

void *multiply( void *ptr );
```

Lalu ada fungsi `multiply` untuk thread perkalian :
```bash
void *multiply( void *ptr )
{
    int ress;
    int *param = (int *)ptr;
    ress = param[0]*param[1];
    
    return (void *) ress;
}
```

Lalu main function :
```bash
int main()
{
    key_t key = 1234;
    int *res;
    int i, j, k;

    int shmid = shmget(key, sizeof(int)*X1*X2, IPC_CREAT | 0666);
    res = shmat(shmid, NULL, 0);

    pthread_t threads[X1][X2][Y];
    int  iret[X1][X2][Y], number1[X1][Y]= {
            {20, 20}, 
            {20, 20}, 
            {20, 20},
            {20, 20}
        }, 
        number2[Y][X2] = {
            {20, 20, 20, 20, 20}, 
            {20, 20, 20, 20, 20}
        };

    for (i = 0; i < X1; i++)
    {
        for (j = 0; j < X2; j++)
        {
            res[j* X2 + i] = 0;
        }
    }
    
    for (i = 0; i < X1; i++)
    {
        for (k = 0; k < X2; k++)
        {
            for (j = 0; j < Y; j++)
            {
                int arr[2] = {number1[i][j], number2[j][k]};

                iret[i][k][j] = pthread_create(&threads[i][k][j], NULL, multiply, (void*) arr);
                
                if(iret[i][k][j]) //jika eror
                {
                    fprintf(stderr,"Error - pthread_create() return code: %d\n",iret[i][k][j]);
                    exit(EXIT_FAILURE);
                }
                
            }    
        }
    }

    for ( i = 0; i < X1; i++)
    {
        for ( k = 0; k < X2; k++)
        {
            for ( j = 0; j < Y; j++)
            {
                void *ress;
                pthread_join(threads[i][k][j], &ress);
                
                res[k*X2 + i] += (int)ress;
            }
        }
    }
    

    for (i = 0; i < X1; i++)
    {
        for ( j = 0; j < X2 - 1; j++)
            printf("%-8d", res[j * X2 + i]);
        printf("%-8d\n", res[j * X2 + X1 - 1]);
    }

    sleep(3);

    shmdt(res);
    shmctl(shmid, IPC_RMID, NULL);
}
```

Lalu untuk `soal4b.c` memiliki header, constant, dan beberapa variabel global seperti :
```bash
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define X1 4
#define X2 5

int result[X1][X2];
void *factorial( void *ptr );
```

Lalu fungsi `factorial` untuk fungsi thread penambahan dari angka 1 sampai angka yang dimasukkan ke fungsi ini :
```bash
void *factorial( void *ptr )
{
    int *param = (int *)ptr, i;

    if(param[0] < 1){
        result[param[1]][param[2]] = 0;
        return NULL;
    }

    result[param[1]][param[2]] = 0;

    for ( i = 1; i <= param[0]; i++)
    {
        result[param[1]][param[2]] += i;
    }

    return NULL;
}
```

Lalu main function :
```bash
int main()
{
    key_t key = 1234;
    int *res; 
    int i, j;

    int shmid = shmget(key, sizeof(int)*X1*X2, IPC_CREAT | 0666), iret[X1][X2];
    res = shmat(shmid, NULL, 0);

    pthread_t threads[X1][X2];

    for (i = 0; i < X1; i++)
    {
        for ( j = 0; j < X2 - 1; j++)
            printf("%-8d", res[j * X2 + i]);
        printf("%-8d\n", res[4 * X2 + i]);
    }
    
    for ( i = 0; i < X1; i++)
    {
        for ( j = 0; j < X2; j++)
            result[i][j] = 0;
    }
    

    for (i = 0; i < X1; i++)
    {
        for (j = 0; j < X2; j++)
        {
            int arr[3] = {res[j * X2 + i], i, j};

            iret[i][j] = pthread_create(&threads[i][j], NULL, factorial, arr);

            if(iret[i][j]) //jika eror
            {
                fprintf(stderr,"Error - pthread_create() return code: %d\n",iret[i][j]);
                exit(EXIT_FAILURE);
            }

            pthread_join(threads[i][j], NULL);
        }
    }

    for (i = 0; i < X1; i++)
    {
        for ( j = 0; j < X2 - 1; j++)
            printf("%-8d", result[i][j]);
        printf("%-8d\n", result[i][X2 - 1]);
    }
    
    sleep(3);

    shmdt(res);
    shmctl(shmid, IPC_RMID, NULL);
}
```

Lalu untuk `soal4c.c` memiliki header dan beberapa constan :
```bash
#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<string.h> 
#include<sys/wait.h> 
#include <fcntl.h> 

#define WRITE 1
#define READ 0
```

Lalu main function :
```bash
int main() 
{ 
	int fd1[2]; 
    int status;

	char input_str[100]; 
	pid_t p;

	if (pipe(fd1) == -1) 
	{ 
		fprintf(stderr, "Pipe Failed" ); 
		return 1; 
	} 

	p = fork(); 

	if (p < 0) 
	{ 
		fprintf(stderr, "fork Failed" ); 
		return 1; 
	}

	if( p == 0 )
	{ 
		int status;

		pid_t child = fork();
		
        if(child == 0) {
			dup2(fd1[WRITE], STDOUT_FILENO);
			close(fd1[READ]);
			char *arr[] = {"ls", NULL};
			execvp("/bin/ls", arr);
		}
		while (wait(&status) > 0);
        
        close(fd1[WRITE]);
        dup2(fd1[READ], STDIN_FILENO);
		char *arr[] = {"wc","-l", NULL};
		execvp("/bin/wc", arr);
	}

	exit(EXIT_SUCCESS);
} 
```

Kesulitan :
- Untuk perkalian masih salah, mungkin karena ada thread yang bertubrukan