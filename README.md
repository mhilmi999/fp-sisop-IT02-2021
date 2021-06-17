## fp-sisop-IT02-2021
---
Penjelasan dan penyelesaian Final Project Sistem Operasi 2021\
Kelompok IT02

1. Muhammad Hilmi Ramadhan (05311940000044)
2. Lambang Akbar Wijayadi (05311940000006)
3. Mulki Kusumah (05311940000043)

---

## Daftar Isi

- [Authentikasi](#authentikasi)
- [Authorisasi](#authorisasi)
- [Data Definition Language](#data-definition-language)
  - [Create Database](#create-database)
  - [Create Table](#create-table)
  - [Drop Database](#drop-database)
  - [Drop Table](#drop-table)
  - [Drop Column](#drop-column)
- [Data Manipulation Language](#data-manipulation-language)
  - [INSERT](#insert)
  - [UPDATE](#update)
  - [DELETE](#delete)
  - [SELECT](#select)
  - [WHERE](#where)
- [Logging](#logging)
- [Reliability](#reliability)
- [Tambahan](#tambahan)
- [Error Handling](#error-handling)


---

<br>



## Authentikasi
---

Source Code tersedia pada : [client.c](./client/client.c)\
Source Code tersedia pada : [dump.c](./client/dump.c)\
Source Code tersedia pada : [database.c](./database/database.c)

## **Analisa Soal**

Secara umum, kami menangkap bahwa program yang harus dibuat merupakan sebuah penerapan dari socket programming dengan bantuan dari thread. Dimana server bisa menerima tidak hanya 1 client request saja. Soal ini akan membuat sebuah database yang dapat diakses oleh pengguna dengan hak tertentu (bergantung `username`, `password`, dan haknya). `root` dapat mengakses seluruh database.

Hal-hal yang perlu diperhatikan diantaranya :
1. Format\
`./client -u username -p password`.
2. *username*, *password*, dan hak akses db disimpan di suatu database juga yang mana tidak akan dapat diakses oleh *user* manapun kecuali `root`.
3. *User* `root` telah ada sejak awal\
Contoh : `sudo ./client`
4. Penambahan *user* baru hanya dapat dilakukan oleh *user* `root` dengan format :\
`CREATE USER [nama_user] IDENTIFIED BY [password_user]`.


<br>

**Cara pengerjaan**
---
Dalam menyelesaikan program yang diminta oleh [Authentikasi](#authentikasi), pertama-tama yang diperlukan adalah melakukan *import* library yang digunakan dalam socket programming :
```cpp
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
```
- `<pthread.h>` library untuk mendapatkan ID dari pemanggilan sebuah thread(e.g. `pthread_self()`)
- `<stdio.h>` library untuk fungsi input-output (e.g. `scanf()`)
- `<sys/types.h>` library digunakan untuk identifikasi sebuah thread (e.g `pthread_t id_thread[3]`)
- `<sys/socket.h>` library untuk membuat socket.
- `<stdlib.h>` library untuk fungsi umum (e.g. `NULL`)
- `<errno.h>` library untuk memberikan tambahan error pada sistem yang sesuai dengan IEEE Std 1003.1-2001  (e.g. `ECHILD`)
- `<string.h>` library untuk melnampilkan pesan error apa saat gagal membuat thread dalam *development side* (e.g. `strerror()`)
- `<arpa/inet.h>` library untuk melakukan koneksi socket
- `<unistd.h>` library untuk mendapatkan lokasi direktori saat bekerja dimana (e.g. `getcwd()`)
- `<netinet/in.h>` library untuk melakukan koneksi socket
- `<sys/stat.h>` library untuk melakukan pemanggilan fungsi dalam pembuatan sebuah direktori baru (e.g. `mkdir()`)
- `<fcntl.h>` library untuk handling file (e.g `open()`)
- `<sys/sendfile.h>` library untuk mengirimkan data antar file descriptor

Selain itu kami sangat terbantu dengan template socket programming yang terdapat dalam modul 3 sistem operasi. Kurang lebih dasar pengerjaan kami adalah modul tersebut yang selanjutnya dikembangkan.

```cpp
int main(int argc, char const *argv[])
{
    getcwd(cwd, sizeof(cwd));
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
```
_Code untuk memulai socket programming dalam server_

```cpp
int main(int argc, char const *argv[])
{
  getcwd(cwd, sizeof(cwd));
  struct sockaddr_in address;
  int sock = 0, valread;
  struct sockaddr_in serv_addr;

  int otentikasi = 0;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Socket creation error \n");
    return -1;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
  {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("\nConnection Failed \n");
    return -1;
  }
```
_Code untuk memulai socket programming dalam client_
<br>
 Untuk melakukan transfer data antara client dan server kami menggunakan potongan program dibawah ini. Untuk mengirim dan menerima pesan dari server dan client kami menggunakan bantuan fungsi `send` untuk melakukan pengiriman buffer data,  `read` untuk menerima dan menyimpan variabel dari data yang dikirimkan, `bzero` untuk me-NULL-kan variabel buffer yang berguna untuk tempat pengiriman ataupun penerimaan data. 

```cpp
    char *message = "user id : ";
    send(new_socket, message, strlen(message), 0);
```
_Code pengiriman data, argumen untuk fungsi `send` ada 4 yaitu `new_socket` sebagai variabel koneksi socket, `message` sebagai pesan yang dikirimkan, `strlen()` sebagai panjangnya data yang dikirimkan , dan 0 sebagai flags default dari fungsi `send`_

```cpp
    int valread;
    char id[1024];
    valread = read(new_socket, id, 1024);
    if (valread < 1)
    {
        printf("eror recv\n");
    }
    printf("%s\n", id);
    strtok(id, "\n");
```
_Code penerimaan data, dimana variabel valread nantinya akan menjadi status keberhasilan penerimaan data, jika fungsi `read` mengembalikan nilai -1 maka akan memunculkan pesan error recv. Fungsi `read` memiliki 3 argument yang pertama `new_socket` berarti file descriptor yang merupakan variabel connect ke server/client, `id` merupakan variabel buffer yang akan menerima pesan yang dikirimkan, dan argumen ketiga yaitu `1024` sebagai panjang file yang mampu ditampung fungsi read._
<br>


## **Kendala**
---

Pada awalnya dari kelompok kami kesulitan untuk mengatasi
<br>


## **Screenshoot Hasil Run Program Authentikasi [Authentikasi](./client/client.c)**
---
![Foto](./img/soal3/bintang.png)
---

<br>

## Authorisasi
---

Source Code tersedia pada : [client.c](./client/client.c)\
Source Code tersedia pada : [dump.c](./client/dump.c)\
Source Code tersedia pada : [database.c](./database/database.c)

## **Analisa Soal**


Hal-hal yang perlu diperhatikan diantaranya :
1. Dalam pengaksesan database yang sesuai dengan `role` nya atau `permission` dengan `command`. Pembuatan tabel dan semua `DML` membutuhkan akses database terlebih dahulu.
\
Format\
`USE[nama_database];`.
2. Pemberian perijinan ataua `permission` hanya dapat dilakukan satu *user* yaitu `root`.\
Format\
`GRANT PERMISSION [nama_database] INTO [nama_user];`
3. User hanya dapat mengakses database dimana dia diberi `permission` untuk database tersebut.


<br>

**Cara pengerjaan**
---


Selain itu kami sangat terbantu dengan template socket programming yang terdapat dalam modul 3 sistem operasi. Kurang lebih dasar pengerjaan kami adalah modul tersebut yang selanjutnya dikembangkan.

```cpp
int main(int argc, char const *argv[])
{
    getcwd(cwd, sizeof(cwd));
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
```
_Code untuk memulai socket programming dalam server_

```cpp
int main(int argc, char const *argv[])
{
  getcwd(cwd, sizeof(cwd));
  struct sockaddr_in address;
  int sock = 0, valread;
  struct sockaddr_in serv_addr;

  int otentikasi = 0;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Socket creation error \n");
    return -1;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
  {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("\nConnection Failed \n");
    return -1;
  }
```
_Code untuk memulai socket programming dalam client_
<br>
 Untuk melakukan transfer data antara client dan server kami menggunakan potongan program dibawah ini. Untuk mengirim dan menerima pesan dari server dan client kami menggunakan bantuan fungsi `send` untuk melakukan pengiriman buffer data,  `read` untuk menerima dan menyimpan variabel dari data yang dikirimkan, `bzero` untuk me-NULL-kan variabel buffer yang berguna untuk tempat pengiriman ataupun penerimaan data. 

```cpp
    char *message = "user id : ";
    send(new_socket, message, strlen(message), 0);
```
_Code pengiriman data, argumen untuk fungsi `send` ada 4 yaitu `new_socket` sebagai variabel koneksi socket, `message` sebagai pesan yang dikirimkan, `strlen()` sebagai panjangnya data yang dikirimkan , dan 0 sebagai flags default dari fungsi `send`_

```cpp
    int valread;
    char id[1024];
    valread = read(new_socket, id, 1024);
    if (valread < 1)
    {
        printf("eror recv\n");
    }
    printf("%s\n", id);
    strtok(id, "\n");
```
_Code penerimaan data, dimana variabel valread nantinya akan menjadi status keberhasilan penerimaan data, jika fungsi `read` mengembalikan nilai -1 maka akan memunculkan pesan error recv. Fungsi `read` memiliki 3 argument yang pertama `new_socket` berarti file descriptor yang merupakan variabel connect ke server/client, `id` merupakan variabel buffer yang akan menerima pesan yang dikirimkan, dan argumen ketiga yaitu `1024` sebagai panjang file yang mampu ditampung fungsi read._
<br>


## **Kendala**
---

Pada awalnya dari kelompok kami kesulitan untuk mengatasi
<br>


## **Screenshoot Hasil Run Program Authorisasi [Authorisasi](./client/client.c)**
---
![Foto](./img/soal3/bintang.png)
---

<br>


## Data Definition Language
---

Source Code tersedia pada : [client.c](./client/client.c)\
Source Code tersedia pada : [dump.c](./client/dump.c)\
Source Code tersedia pada : [database.c](./database/database.c)

## **Analisa Soal**

Hal-hal yang perlu diperhatikan diantaranya :
1. `Input` penamaan database, tabel, dan kolom hanya angka dan huruf.
2. Semua *user* dapat membuat database, otomatis *user* tersebut memiliki `permission` untuk database tersebut.\
Format\
`CREATE DATABASE [nama_database];`
3. `root` dan `user` yang memiliki `permission` untuk sebuah database dapat membuat tabel untuk database tersebut, tentunya setelah mengakses database tersebut. Tipe data dari semua kolom adalah string atau integer dengan jumlah kolom bebas.\
Format\
`CREATE TABLE [nama_table] ([nama_kolom] [tipe_data],...);`
4. Dapat melakukan `DROP` database, table (ketika telah mengaksesnya), dan kolom. Jika *target* `DROP` ada maka akan tereksekusi, namun sebaliknya jika tidak ada maka dibiarkan.\
Format\
`DROP [DATABASE | TABLE | COLUMN] [nama_database | nama_table | [nama_kolom] FROM [nama_table];`

<br>

**Cara pengerjaan**
---

## Create Database
Selain itu kami sangat terbantu dengan template socket programming yang terdapat dalam modul 3 sistem operasi. Kurang lebih dasar pengerjaan kami adalah modul tersebut yang selanjutnya dikembangkan.

```cpp
int main(int argc, char const *argv[])
{
    getcwd(cwd, sizeof(cwd));
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
```
_Code untuk memulai socket programming dalam server_

```cpp
int main(int argc, char const *argv[])
{
  getcwd(cwd, sizeof(cwd));
  struct sockaddr_in address;
  int sock = 0, valread;
  struct sockaddr_in serv_addr;

  int otentikasi = 0;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Socket creation error \n");
    return -1;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
  {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("\nConnection Failed \n");
    return -1;
  }
```
_Code untuk memulai socket programming dalam client_
<br>
 Untuk melakukan transfer data antara client dan server kami menggunakan potongan program dibawah ini. Untuk mengirim dan menerima pesan dari server dan client kami menggunakan bantuan fungsi `send` untuk melakukan pengiriman buffer data,  `read` untuk menerima dan menyimpan variabel dari data yang dikirimkan, `bzero` untuk me-NULL-kan variabel buffer yang berguna untuk tempat pengiriman ataupun penerimaan data. 

```cpp
    char *message = "user id : ";
    send(new_socket, message, strlen(message), 0);
```
_Code pengiriman data, argumen untuk fungsi `send` ada 4 yaitu `new_socket` sebagai variabel koneksi socket, `message` sebagai pesan yang dikirimkan, `strlen()` sebagai panjangnya data yang dikirimkan , dan 0 sebagai flags default dari fungsi `send`_

```cpp
    int valread;
    char id[1024];
    valread = read(new_socket, id, 1024);
    if (valread < 1)
    {
        printf("eror recv\n");
    }
    printf("%s\n", id);
    strtok(id, "\n");
```
_Code penerimaan data, dimana variabel valread nantinya akan menjadi status keberhasilan penerimaan data, jika fungsi `read` mengembalikan nilai -1 maka akan memunculkan pesan error recv. Fungsi `read` memiliki 3 argument yang pertama `new_socket` berarti file descriptor yang merupakan variabel connect ke server/client, `id` merupakan variabel buffer yang akan menerima pesan yang dikirimkan, dan argumen ketiga yaitu `1024` sebagai panjang file yang mampu ditampung fungsi read._
<br>

## **Screenshoot Hasil Run Program Data Definition Language CREATE DATABASE [Authorisasi](./client/client.c)**
---
![Foto](./img/soal3/bintang.png)
---

<br>

## Create Database
Selain itu kami sangat terbantu dengan template socket programming yang terdapat dalam modul 3 sistem operasi. Kurang lebih dasar pengerjaan kami adalah modul tersebut yang selanjutnya dikembangkan.

```cpp
int main(int argc, char const *argv[])
{
    getcwd(cwd, sizeof(cwd));
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
```
_Code untuk memulai socket programming dalam server_

```cpp
int main(int argc, char const *argv[])
{
  getcwd(cwd, sizeof(cwd));
  struct sockaddr_in address;
  int sock = 0, valread;
  struct sockaddr_in serv_addr;

  int otentikasi = 0;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Socket creation error \n");
    return -1;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
  {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("\nConnection Failed \n");
    return -1;
  }
```
_Code untuk memulai socket programming dalam client_
<br>
 Untuk melakukan transfer data antara client dan server kami menggunakan potongan program dibawah ini. Untuk mengirim dan menerima pesan dari server dan client kami menggunakan bantuan fungsi `send` untuk melakukan pengiriman buffer data,  `read` untuk menerima dan menyimpan variabel dari data yang dikirimkan, `bzero` untuk me-NULL-kan variabel buffer yang berguna untuk tempat pengiriman ataupun penerimaan data. 

```cpp
    char *message = "user id : ";
    send(new_socket, message, strlen(message), 0);
```
_Code pengiriman data, argumen untuk fungsi `send` ada 4 yaitu `new_socket` sebagai variabel koneksi socket, `message` sebagai pesan yang dikirimkan, `strlen()` sebagai panjangnya data yang dikirimkan , dan 0 sebagai flags default dari fungsi `send`_

```cpp
    int valread;
    char id[1024];
    valread = read(new_socket, id, 1024);
    if (valread < 1)
    {
        printf("eror recv\n");
    }
    printf("%s\n", id);
    strtok(id, "\n");
```
_Code penerimaan data, dimana variabel valread nantinya akan menjadi status keberhasilan penerimaan data, jika fungsi `read` mengembalikan nilai -1 maka akan memunculkan pesan error recv. Fungsi `read` memiliki 3 argument yang pertama `new_socket` berarti file descriptor yang merupakan variabel connect ke server/client, `id` merupakan variabel buffer yang akan menerima pesan yang dikirimkan, dan argumen ketiga yaitu `1024` sebagai panjang file yang mampu ditampung fungsi read._
<br>

## **Screenshoot Hasil Run Program Data Definition Language CREATE DATABASE [Authorisasi](./client/client.c)**
---
![Foto](./img/soal3/bintang.png)
---

<br>

## Create Table
Selain itu kami sangat terbantu dengan template socket programming yang terdapat dalam modul 3 sistem operasi. Kurang lebih dasar pengerjaan kami adalah modul tersebut yang selanjutnya dikembangkan.

```cpp
int main(int argc, char const *argv[])
{
    getcwd(cwd, sizeof(cwd));
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
```
_Code untuk memulai socket programming dalam server_

```cpp
int main(int argc, char const *argv[])
{
  getcwd(cwd, sizeof(cwd));
  struct sockaddr_in address;
  int sock = 0, valread;
  struct sockaddr_in serv_addr;

  int otentikasi = 0;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Socket creation error \n");
    return -1;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
  {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("\nConnection Failed \n");
    return -1;
  }
```
_Code untuk memulai socket programming dalam client_
<br>
 Untuk melakukan transfer data antara client dan server kami menggunakan potongan program dibawah ini. Untuk mengirim dan menerima pesan dari server dan client kami menggunakan bantuan fungsi `send` untuk melakukan pengiriman buffer data,  `read` untuk menerima dan menyimpan variabel dari data yang dikirimkan, `bzero` untuk me-NULL-kan variabel buffer yang berguna untuk tempat pengiriman ataupun penerimaan data. 

```cpp
    char *message = "user id : ";
    send(new_socket, message, strlen(message), 0);
```
_Code pengiriman data, argumen untuk fungsi `send` ada 4 yaitu `new_socket` sebagai variabel koneksi socket, `message` sebagai pesan yang dikirimkan, `strlen()` sebagai panjangnya data yang dikirimkan , dan 0 sebagai flags default dari fungsi `send`_

```cpp
    int valread;
    char id[1024];
    valread = read(new_socket, id, 1024);
    if (valread < 1)
    {
        printf("eror recv\n");
    }
    printf("%s\n", id);
    strtok(id, "\n");
```
_Code penerimaan data, dimana variabel valread nantinya akan menjadi status keberhasilan penerimaan data, jika fungsi `read` mengembalikan nilai -1 maka akan memunculkan pesan error recv. Fungsi `read` memiliki 3 argument yang pertama `new_socket` berarti file descriptor yang merupakan variabel connect ke server/client, `id` merupakan variabel buffer yang akan menerima pesan yang dikirimkan, dan argumen ketiga yaitu `1024` sebagai panjang file yang mampu ditampung fungsi read._
<br>

## **Screenshoot Hasil Run Program Data Definition Language CREATE TABLE [Authorisasi](./client/client.c)**
---
![Foto](./img/soal3/bintang.png)
---

<br>

## Drop Database
Selain itu kami sangat terbantu dengan template socket programming yang terdapat dalam modul 3 sistem operasi. Kurang lebih dasar pengerjaan kami adalah modul tersebut yang selanjutnya dikembangkan.

```cpp
int main(int argc, char const *argv[])
{
    getcwd(cwd, sizeof(cwd));
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
```
_Code untuk memulai socket programming dalam server_

```cpp
int main(int argc, char const *argv[])
{
  getcwd(cwd, sizeof(cwd));
  struct sockaddr_in address;
  int sock = 0, valread;
  struct sockaddr_in serv_addr;

  int otentikasi = 0;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Socket creation error \n");
    return -1;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
  {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("\nConnection Failed \n");
    return -1;
  }
```
_Code untuk memulai socket programming dalam client_
<br>
 Untuk melakukan transfer data antara client dan server kami menggunakan potongan program dibawah ini. Untuk mengirim dan menerima pesan dari server dan client kami menggunakan bantuan fungsi `send` untuk melakukan pengiriman buffer data,  `read` untuk menerima dan menyimpan variabel dari data yang dikirimkan, `bzero` untuk me-NULL-kan variabel buffer yang berguna untuk tempat pengiriman ataupun penerimaan data. 

```cpp
    char *message = "user id : ";
    send(new_socket, message, strlen(message), 0);
```
_Code pengiriman data, argumen untuk fungsi `send` ada 4 yaitu `new_socket` sebagai variabel koneksi socket, `message` sebagai pesan yang dikirimkan, `strlen()` sebagai panjangnya data yang dikirimkan , dan 0 sebagai flags default dari fungsi `send`_

```cpp
    int valread;
    char id[1024];
    valread = read(new_socket, id, 1024);
    if (valread < 1)
    {
        printf("eror recv\n");
    }
    printf("%s\n", id);
    strtok(id, "\n");
```
_Code penerimaan data, dimana variabel valread nantinya akan menjadi status keberhasilan penerimaan data, jika fungsi `read` mengembalikan nilai -1 maka akan memunculkan pesan error recv. Fungsi `read` memiliki 3 argument yang pertama `new_socket` berarti file descriptor yang merupakan variabel connect ke server/client, `id` merupakan variabel buffer yang akan menerima pesan yang dikirimkan, dan argumen ketiga yaitu `1024` sebagai panjang file yang mampu ditampung fungsi read._
<br>

## **Screenshoot Hasil Run Program Data Definition Language DROP DATABASE [Authorisasi](./client/client.c)**
---
![Foto](./img/soal3/bintang.png)
---

<br>

## Drop Table
Selain itu kami sangat terbantu dengan template socket programming yang terdapat dalam modul 3 sistem operasi. Kurang lebih dasar pengerjaan kami adalah modul tersebut yang selanjutnya dikembangkan.

```cpp
int main(int argc, char const *argv[])
{
    getcwd(cwd, sizeof(cwd));
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
```
_Code untuk memulai socket programming dalam server_

```cpp
int main(int argc, char const *argv[])
{
  getcwd(cwd, sizeof(cwd));
  struct sockaddr_in address;
  int sock = 0, valread;
  struct sockaddr_in serv_addr;

  int otentikasi = 0;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Socket creation error \n");
    return -1;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
  {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("\nConnection Failed \n");
    return -1;
  }
```
_Code untuk memulai socket programming dalam client_
<br>
 Untuk melakukan transfer data antara client dan server kami menggunakan potongan program dibawah ini. Untuk mengirim dan menerima pesan dari server dan client kami menggunakan bantuan fungsi `send` untuk melakukan pengiriman buffer data,  `read` untuk menerima dan menyimpan variabel dari data yang dikirimkan, `bzero` untuk me-NULL-kan variabel buffer yang berguna untuk tempat pengiriman ataupun penerimaan data. 

```cpp
    char *message = "user id : ";
    send(new_socket, message, strlen(message), 0);
```
_Code pengiriman data, argumen untuk fungsi `send` ada 4 yaitu `new_socket` sebagai variabel koneksi socket, `message` sebagai pesan yang dikirimkan, `strlen()` sebagai panjangnya data yang dikirimkan , dan 0 sebagai flags default dari fungsi `send`_

```cpp
    int valread;
    char id[1024];
    valread = read(new_socket, id, 1024);
    if (valread < 1)
    {
        printf("eror recv\n");
    }
    printf("%s\n", id);
    strtok(id, "\n");
```
_Code penerimaan data, dimana variabel valread nantinya akan menjadi status keberhasilan penerimaan data, jika fungsi `read` mengembalikan nilai -1 maka akan memunculkan pesan error recv. Fungsi `read` memiliki 3 argument yang pertama `new_socket` berarti file descriptor yang merupakan variabel connect ke server/client, `id` merupakan variabel buffer yang akan menerima pesan yang dikirimkan, dan argumen ketiga yaitu `1024` sebagai panjang file yang mampu ditampung fungsi read._
<br>

## **Screenshoot Hasil Run Program Data Definition Language DROP Table [Authorisasi](./client/client.c)**
---
![Foto](./img/soal3/bintang.png)
---

<br>

## Drop Column
Selain itu kami sangat terbantu dengan template socket programming yang terdapat dalam modul 3 sistem operasi. Kurang lebih dasar pengerjaan kami adalah modul tersebut yang selanjutnya dikembangkan.

```cpp
int main(int argc, char const *argv[])
{
    getcwd(cwd, sizeof(cwd));
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
```
_Code untuk memulai socket programming dalam server_

```cpp
int main(int argc, char const *argv[])
{
  getcwd(cwd, sizeof(cwd));
  struct sockaddr_in address;
  int sock = 0, valread;
  struct sockaddr_in serv_addr;

  int otentikasi = 0;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Socket creation error \n");
    return -1;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
  {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("\nConnection Failed \n");
    return -1;
  }
```
_Code untuk memulai socket programming dalam client_
<br>
 Untuk melakukan transfer data antara client dan server kami menggunakan potongan program dibawah ini. Untuk mengirim dan menerima pesan dari server dan client kami menggunakan bantuan fungsi `send` untuk melakukan pengiriman buffer data,  `read` untuk menerima dan menyimpan variabel dari data yang dikirimkan, `bzero` untuk me-NULL-kan variabel buffer yang berguna untuk tempat pengiriman ataupun penerimaan data. 

```cpp
    char *message = "user id : ";
    send(new_socket, message, strlen(message), 0);
```
_Code pengiriman data, argumen untuk fungsi `send` ada 4 yaitu `new_socket` sebagai variabel koneksi socket, `message` sebagai pesan yang dikirimkan, `strlen()` sebagai panjangnya data yang dikirimkan , dan 0 sebagai flags default dari fungsi `send`_

```cpp
    int valread;
    char id[1024];
    valread = read(new_socket, id, 1024);
    if (valread < 1)
    {
        printf("eror recv\n");
    }
    printf("%s\n", id);
    strtok(id, "\n");
```
_Code penerimaan data, dimana variabel valread nantinya akan menjadi status keberhasilan penerimaan data, jika fungsi `read` mengembalikan nilai -1 maka akan memunculkan pesan error recv. Fungsi `read` memiliki 3 argument yang pertama `new_socket` berarti file descriptor yang merupakan variabel connect ke server/client, `id` merupakan variabel buffer yang akan menerima pesan yang dikirimkan, dan argumen ketiga yaitu `1024` sebagai panjang file yang mampu ditampung fungsi read._
<br>

## **Screenshoot Hasil Run Program Data Definition Language DROP COLUMN [Authorisasi](./client/client.c)**
---
![Foto](./img/soal3/bintang.png)
---

<br>


## Data Manipulation Language
---

Source Code tersedia pada : [client.c](./client/client.c)\
Source Code tersedia pada : [dump.c](./client/dump.c)\
Source Code tersedia pada : [database.c](./database/database.c)

## **Analisa Soal**

Hal-hal yang perlu diperhatikan diantaranya :
1. `INSERT`\
Hanya dapat `INSERT` satu `row` per satu `command`. `Insert` sesuai dengan jumlah dan urutan kolom.\
Format\
`INSERT INTO [nama_tabel] ([value], ...);`
2. `UPDATE`\
Hanya dapat melakukan perubahan dalam satu kolom tiap satu `command`.\
Format\
`UPDATE [nama_table] SET [nama_kolom] = [value];`
3. `DELETE`\
Menghapus data yang ada pada tabel.\
Format\
`DELETE FROM [nama_tabel];`
4. `SELECT`\
Format\
`SELECT [nama_kolom, ... | *] FROM [nama_tabel];`
5. `WHERE`\
*Command* `UPDATE`, `SELECT`, dan `DELETE` dapat dikombinasikan dengan `WHERE`. Hal ini hanya berlaku dalam satu kondisi serta dan hanya `'='`.\
Format\
`[Commmand UPDATE, SELECT, DELETE] WHERE [nama_kolom] = [value];`

<br>

**Cara pengerjaan**
---

## Insert
Selain itu kami sangat terbantu dengan template socket programming yang terdapat dalam modul 3 sistem operasi. Kurang lebih dasar pengerjaan kami adalah modul tersebut yang selanjutnya dikembangkan.

```cpp
int main(int argc, char const *argv[])
{
    getcwd(cwd, sizeof(cwd));
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
```
_Code untuk memulai socket programming dalam server_

```cpp
int main(int argc, char const *argv[])
{
  getcwd(cwd, sizeof(cwd));
  struct sockaddr_in address;
  int sock = 0, valread;
  struct sockaddr_in serv_addr;

  int otentikasi = 0;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Socket creation error \n");
    return -1;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
  {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("\nConnection Failed \n");
    return -1;
  }
```
_Code untuk memulai socket programming dalam client_
<br>
 Untuk melakukan transfer data antara client dan server kami menggunakan potongan program dibawah ini. Untuk mengirim dan menerima pesan dari server dan client kami menggunakan bantuan fungsi `send` untuk melakukan pengiriman buffer data,  `read` untuk menerima dan menyimpan variabel dari data yang dikirimkan, `bzero` untuk me-NULL-kan variabel buffer yang berguna untuk tempat pengiriman ataupun penerimaan data. 

```cpp
    char *message = "user id : ";
    send(new_socket, message, strlen(message), 0);
```
_Code pengiriman data, argumen untuk fungsi `send` ada 4 yaitu `new_socket` sebagai variabel koneksi socket, `message` sebagai pesan yang dikirimkan, `strlen()` sebagai panjangnya data yang dikirimkan , dan 0 sebagai flags default dari fungsi `send`_

```cpp
    int valread;
    char id[1024];
    valread = read(new_socket, id, 1024);
    if (valread < 1)
    {
        printf("eror recv\n");
    }
    printf("%s\n", id);
    strtok(id, "\n");
```
_Code penerimaan data, dimana variabel valread nantinya akan menjadi status keberhasilan penerimaan data, jika fungsi `read` mengembalikan nilai -1 maka akan memunculkan pesan error recv. Fungsi `read` memiliki 3 argument yang pertama `new_socket` berarti file descriptor yang merupakan variabel connect ke server/client, `id` merupakan variabel buffer yang akan menerima pesan yang dikirimkan, dan argumen ketiga yaitu `1024` sebagai panjang file yang mampu ditampung fungsi read._
<br>

## **Screenshoot Hasil Run Program Data Definition Language CREATE DATABASE [Authorisasi](./client/client.c)**
---
![Foto](./img/soal3/bintang.png)
---

<br>

## Update
Selain itu kami sangat terbantu dengan template socket programming yang terdapat dalam modul 3 sistem operasi. Kurang lebih dasar pengerjaan kami adalah modul tersebut yang selanjutnya dikembangkan.

```cpp
int main(int argc, char const *argv[])
{
    getcwd(cwd, sizeof(cwd));
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
```
_Code untuk memulai socket programming dalam server_

```cpp
int main(int argc, char const *argv[])
{
  getcwd(cwd, sizeof(cwd));
  struct sockaddr_in address;
  int sock = 0, valread;
  struct sockaddr_in serv_addr;

  int otentikasi = 0;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Socket creation error \n");
    return -1;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
  {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("\nConnection Failed \n");
    return -1;
  }
```
_Code untuk memulai socket programming dalam client_
<br>
 Untuk melakukan transfer data antara client dan server kami menggunakan potongan program dibawah ini. Untuk mengirim dan menerima pesan dari server dan client kami menggunakan bantuan fungsi `send` untuk melakukan pengiriman buffer data,  `read` untuk menerima dan menyimpan variabel dari data yang dikirimkan, `bzero` untuk me-NULL-kan variabel buffer yang berguna untuk tempat pengiriman ataupun penerimaan data. 

```cpp
    char *message = "user id : ";
    send(new_socket, message, strlen(message), 0);
```
_Code pengiriman data, argumen untuk fungsi `send` ada 4 yaitu `new_socket` sebagai variabel koneksi socket, `message` sebagai pesan yang dikirimkan, `strlen()` sebagai panjangnya data yang dikirimkan , dan 0 sebagai flags default dari fungsi `send`_

```cpp
    int valread;
    char id[1024];
    valread = read(new_socket, id, 1024);
    if (valread < 1)
    {
        printf("eror recv\n");
    }
    printf("%s\n", id);
    strtok(id, "\n");
```
_Code penerimaan data, dimana variabel valread nantinya akan menjadi status keberhasilan penerimaan data, jika fungsi `read` mengembalikan nilai -1 maka akan memunculkan pesan error recv. Fungsi `read` memiliki 3 argument yang pertama `new_socket` berarti file descriptor yang merupakan variabel connect ke server/client, `id` merupakan variabel buffer yang akan menerima pesan yang dikirimkan, dan argumen ketiga yaitu `1024` sebagai panjang file yang mampu ditampung fungsi read._
<br>

## **Screenshoot Hasil Run Program Data Definition Language CREATE DATABASE [Authorisasi](./client/client.c)**
---
![Foto](./img/soal3/bintang.png)
---

<br>

## Delete
Selain itu kami sangat terbantu dengan template socket programming yang terdapat dalam modul 3 sistem operasi. Kurang lebih dasar pengerjaan kami adalah modul tersebut yang selanjutnya dikembangkan.

```cpp
int main(int argc, char const *argv[])
{
    getcwd(cwd, sizeof(cwd));
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
```
_Code untuk memulai socket programming dalam server_

```cpp
int main(int argc, char const *argv[])
{
  getcwd(cwd, sizeof(cwd));
  struct sockaddr_in address;
  int sock = 0, valread;
  struct sockaddr_in serv_addr;

  int otentikasi = 0;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Socket creation error \n");
    return -1;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
  {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("\nConnection Failed \n");
    return -1;
  }
```
_Code untuk memulai socket programming dalam client_
<br>
 Untuk melakukan transfer data antara client dan server kami menggunakan potongan program dibawah ini. Untuk mengirim dan menerima pesan dari server dan client kami menggunakan bantuan fungsi `send` untuk melakukan pengiriman buffer data,  `read` untuk menerima dan menyimpan variabel dari data yang dikirimkan, `bzero` untuk me-NULL-kan variabel buffer yang berguna untuk tempat pengiriman ataupun penerimaan data. 

```cpp
    char *message = "user id : ";
    send(new_socket, message, strlen(message), 0);
```
_Code pengiriman data, argumen untuk fungsi `send` ada 4 yaitu `new_socket` sebagai variabel koneksi socket, `message` sebagai pesan yang dikirimkan, `strlen()` sebagai panjangnya data yang dikirimkan , dan 0 sebagai flags default dari fungsi `send`_

```cpp
    int valread;
    char id[1024];
    valread = read(new_socket, id, 1024);
    if (valread < 1)
    {
        printf("eror recv\n");
    }
    printf("%s\n", id);
    strtok(id, "\n");
```
_Code penerimaan data, dimana variabel valread nantinya akan menjadi status keberhasilan penerimaan data, jika fungsi `read` mengembalikan nilai -1 maka akan memunculkan pesan error recv. Fungsi `read` memiliki 3 argument yang pertama `new_socket` berarti file descriptor yang merupakan variabel connect ke server/client, `id` merupakan variabel buffer yang akan menerima pesan yang dikirimkan, dan argumen ketiga yaitu `1024` sebagai panjang file yang mampu ditampung fungsi read._
<br>

## **Screenshoot Hasil Run Program Data Definition Language CREATE TABLE [Authorisasi](./client/client.c)**
---
![Foto](./img/soal3/bintang.png)
---

<br>

## Select
Selain itu kami sangat terbantu dengan template socket programming yang terdapat dalam modul 3 sistem operasi. Kurang lebih dasar pengerjaan kami adalah modul tersebut yang selanjutnya dikembangkan.

```cpp
int main(int argc, char const *argv[])
{
    getcwd(cwd, sizeof(cwd));
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
```
_Code untuk memulai socket programming dalam server_

```cpp
int main(int argc, char const *argv[])
{
  getcwd(cwd, sizeof(cwd));
  struct sockaddr_in address;
  int sock = 0, valread;
  struct sockaddr_in serv_addr;

  int otentikasi = 0;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Socket creation error \n");
    return -1;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
  {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("\nConnection Failed \n");
    return -1;
  }
```
_Code untuk memulai socket programming dalam client_
<br>
 Untuk melakukan transfer data antara client dan server kami menggunakan potongan program dibawah ini. Untuk mengirim dan menerima pesan dari server dan client kami menggunakan bantuan fungsi `send` untuk melakukan pengiriman buffer data,  `read` untuk menerima dan menyimpan variabel dari data yang dikirimkan, `bzero` untuk me-NULL-kan variabel buffer yang berguna untuk tempat pengiriman ataupun penerimaan data. 

```cpp
    char *message = "user id : ";
    send(new_socket, message, strlen(message), 0);
```
_Code pengiriman data, argumen untuk fungsi `send` ada 4 yaitu `new_socket` sebagai variabel koneksi socket, `message` sebagai pesan yang dikirimkan, `strlen()` sebagai panjangnya data yang dikirimkan , dan 0 sebagai flags default dari fungsi `send`_

```cpp
    int valread;
    char id[1024];
    valread = read(new_socket, id, 1024);
    if (valread < 1)
    {
        printf("eror recv\n");
    }
    printf("%s\n", id);
    strtok(id, "\n");
```
_Code penerimaan data, dimana variabel valread nantinya akan menjadi status keberhasilan penerimaan data, jika fungsi `read` mengembalikan nilai -1 maka akan memunculkan pesan error recv. Fungsi `read` memiliki 3 argument yang pertama `new_socket` berarti file descriptor yang merupakan variabel connect ke server/client, `id` merupakan variabel buffer yang akan menerima pesan yang dikirimkan, dan argumen ketiga yaitu `1024` sebagai panjang file yang mampu ditampung fungsi read._
<br>

## **Screenshoot Hasil Run Program Data Definition Language DROP DATABASE [Authorisasi](./client/client.c)**
---
![Foto](./img/soal3/bintang.png)
---

<br>

## Where
Selain itu kami sangat terbantu dengan template socket programming yang terdapat dalam modul 3 sistem operasi. Kurang lebih dasar pengerjaan kami adalah modul tersebut yang selanjutnya dikembangkan.

```cpp
int main(int argc, char const *argv[])
{
    getcwd(cwd, sizeof(cwd));
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
```
_Code untuk memulai socket programming dalam server_

```cpp
int main(int argc, char const *argv[])
{
  getcwd(cwd, sizeof(cwd));
  struct sockaddr_in address;
  int sock = 0, valread;
  struct sockaddr_in serv_addr;

  int otentikasi = 0;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Socket creation error \n");
    return -1;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
  {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("\nConnection Failed \n");
    return -1;
  }
```
_Code untuk memulai socket programming dalam client_
<br>
 Untuk melakukan transfer data antara client dan server kami menggunakan potongan program dibawah ini. Untuk mengirim dan menerima pesan dari server dan client kami menggunakan bantuan fungsi `send` untuk melakukan pengiriman buffer data,  `read` untuk menerima dan menyimpan variabel dari data yang dikirimkan, `bzero` untuk me-NULL-kan variabel buffer yang berguna untuk tempat pengiriman ataupun penerimaan data. 

```cpp
    char *message = "user id : ";
    send(new_socket, message, strlen(message), 0);
```
_Code pengiriman data, argumen untuk fungsi `send` ada 4 yaitu `new_socket` sebagai variabel koneksi socket, `message` sebagai pesan yang dikirimkan, `strlen()` sebagai panjangnya data yang dikirimkan , dan 0 sebagai flags default dari fungsi `send`_

```cpp
    int valread;
    char id[1024];
    valread = read(new_socket, id, 1024);
    if (valread < 1)
    {
        printf("eror recv\n");
    }
    printf("%s\n", id);
    strtok(id, "\n");
```
_Code penerimaan data, dimana variabel valread nantinya akan menjadi status keberhasilan penerimaan data, jika fungsi `read` mengembalikan nilai -1 maka akan memunculkan pesan error recv. Fungsi `read` memiliki 3 argument yang pertama `new_socket` berarti file descriptor yang merupakan variabel connect ke server/client, `id` merupakan variabel buffer yang akan menerima pesan yang dikirimkan, dan argumen ketiga yaitu `1024` sebagai panjang file yang mampu ditampung fungsi read._
<br>

## **Screenshoot Hasil Run Program Data Definition Language DROP Table [Authorisasi](./client/client.c)**
---
![Foto](./img/soal3/bintang.png)
---

<br>


## Logging
---

Source Code tersedia pada : [client.c](./client/client.c)\
Source Code tersedia pada : [dump.c](./client/dump.c)\
Source Code tersedia pada : [database.c](./database/database.c)

## **Analisa Soal**


Hal-hal yang perlu diperhatikan diantaranya :
1. Setiap `command` yang dipakai harus dilakukan *logging* ke suatu file dengan format. Apabila yang melakukan eksekusi `root` maka `username` `root`.
\
Format di dalam log\
`timestamp(yyyy-mm-dd hh:mm:ss):username:command`.
2. Contoh\
`2021-05-19 02:05:15:jack:SELECT FROM table1`

<br>

**Cara pengerjaan**
---


Selain itu kami sangat terbantu dengan template socket programming yang terdapat dalam modul 3 sistem operasi. Kurang lebih dasar pengerjaan kami adalah modul tersebut yang selanjutnya dikembangkan.

```cpp
int main(int argc, char const *argv[])
{
    getcwd(cwd, sizeof(cwd));
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
```
_Code untuk memulai socket programming dalam server_

```cpp
int main(int argc, char const *argv[])
{
  getcwd(cwd, sizeof(cwd));
  struct sockaddr_in address;
  int sock = 0, valread;
  struct sockaddr_in serv_addr;

  int otentikasi = 0;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Socket creation error \n");
    return -1;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
  {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("\nConnection Failed \n");
    return -1;
  }
```
_Code untuk memulai socket programming dalam client_
<br>
 Untuk melakukan transfer data antara client dan server kami menggunakan potongan program dibawah ini. Untuk mengirim dan menerima pesan dari server dan client kami menggunakan bantuan fungsi `send` untuk melakukan pengiriman buffer data,  `read` untuk menerima dan menyimpan variabel dari data yang dikirimkan, `bzero` untuk me-NULL-kan variabel buffer yang berguna untuk tempat pengiriman ataupun penerimaan data. 

```cpp
    char *message = "user id : ";
    send(new_socket, message, strlen(message), 0);
```
_Code pengiriman data, argumen untuk fungsi `send` ada 4 yaitu `new_socket` sebagai variabel koneksi socket, `message` sebagai pesan yang dikirimkan, `strlen()` sebagai panjangnya data yang dikirimkan , dan 0 sebagai flags default dari fungsi `send`_

```cpp
    int valread;
    char id[1024];
    valread = read(new_socket, id, 1024);
    if (valread < 1)
    {
        printf("eror recv\n");
    }
    printf("%s\n", id);
    strtok(id, "\n");
```
_Code penerimaan data, dimana variabel valread nantinya akan menjadi status keberhasilan penerimaan data, jika fungsi `read` mengembalikan nilai -1 maka akan memunculkan pesan error recv. Fungsi `read` memiliki 3 argument yang pertama `new_socket` berarti file descriptor yang merupakan variabel connect ke server/client, `id` merupakan variabel buffer yang akan menerima pesan yang dikirimkan, dan argumen ketiga yaitu `1024` sebagai panjang file yang mampu ditampung fungsi read._
<br>


## **Kendala**
---

Pada awalnya dari kelompok kami kesulitan untuk mengatasi
<br>


## **Screenshoot Hasil Run Program Logging [Logging](./client/client.c)**
---
![Foto](./img/soal3/bintang.png)
---

<br>


## Reliability
---

Source Code tersedia pada : [client.c](./client/client.c)\
Source Code tersedia pada : [dump.c](./client/dump.c)\
Source Code tersedia pada : [database.c](./database/database.c)

## **Analisa Soal**


Hal-hal yang perlu diperhatikan diantaranya :
1. Harus membuat suatu program terpisah untuk dump database ke command-command yang akan di `print ke layar`. Untuk memasukkan ke file, gunakan redirection. Program ini tentunya harus melalui proses autentikasi terlebih dahulu. Ini sampai database level saja, tidak perlu sampai tabel.
\
Format\
`./[program_dump_database] -u [username] -p [password] [nama_database]`.


<br>

**Cara pengerjaan**
---


Selain itu kami sangat terbantu dengan template socket programming yang terdapat dalam modul 3 sistem operasi. Kurang lebih dasar pengerjaan kami adalah modul tersebut yang selanjutnya dikembangkan.

```cpp
int main(int argc, char const *argv[])
{
    getcwd(cwd, sizeof(cwd));
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
```
_Code untuk memulai socket programming dalam server_

```cpp
int main(int argc, char const *argv[])
{
  getcwd(cwd, sizeof(cwd));
  struct sockaddr_in address;
  int sock = 0, valread;
  struct sockaddr_in serv_addr;

  int otentikasi = 0;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Socket creation error \n");
    return -1;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
  {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("\nConnection Failed \n");
    return -1;
  }
```
_Code untuk memulai socket programming dalam client_
<br>
 Untuk melakukan transfer data antara client dan server kami menggunakan potongan program dibawah ini. Untuk mengirim dan menerima pesan dari server dan client kami menggunakan bantuan fungsi `send` untuk melakukan pengiriman buffer data,  `read` untuk menerima dan menyimpan variabel dari data yang dikirimkan, `bzero` untuk me-NULL-kan variabel buffer yang berguna untuk tempat pengiriman ataupun penerimaan data. 

```cpp
    char *message = "user id : ";
    send(new_socket, message, strlen(message), 0);
```
_Code pengiriman data, argumen untuk fungsi `send` ada 4 yaitu `new_socket` sebagai variabel koneksi socket, `message` sebagai pesan yang dikirimkan, `strlen()` sebagai panjangnya data yang dikirimkan , dan 0 sebagai flags default dari fungsi `send`_

```cpp
    int valread;
    char id[1024];
    valread = read(new_socket, id, 1024);
    if (valread < 1)
    {
        printf("eror recv\n");
    }
    printf("%s\n", id);
    strtok(id, "\n");
```
_Code penerimaan data, dimana variabel valread nantinya akan menjadi status keberhasilan penerimaan data, jika fungsi `read` mengembalikan nilai -1 maka akan memunculkan pesan error recv. Fungsi `read` memiliki 3 argument yang pertama `new_socket` berarti file descriptor yang merupakan variabel connect ke server/client, `id` merupakan variabel buffer yang akan menerima pesan yang dikirimkan, dan argumen ketiga yaitu `1024` sebagai panjang file yang mampu ditampung fungsi read._
<br>


## **Kendala**
---

Pada awalnya dari kelompok kami kesulitan untuk mengatasi
<br>


## **Screenshoot Hasil Run Program Reliability [Reliability](./client/client.c)**
---
![Foto](./img/soal3/bintang.png)
---

<br>


## Tambahan
---

Source Code tersedia pada : [client.c](./client/client.c)\
Source Code tersedia pada : [dump.c](./client/dump.c)\
Source Code tersedia pada : [database.c](./database/database.c)

## **Analisa Soal**


Hal-hal yang perlu diperhatikan diantaranya :
1. Kita bisa memasukkan `command` lewat file dengan *redirection* di program `client`.
\
Format\
`./[program_client_database] -u [username] -p [password] -d [database] < [file_command]`.


<br>

**Cara pengerjaan**
---


Selain itu kami sangat terbantu dengan template socket programming yang terdapat dalam modul 3 sistem operasi. Kurang lebih dasar pengerjaan kami adalah modul tersebut yang selanjutnya dikembangkan.

```cpp
int main(int argc, char const *argv[])
{
    getcwd(cwd, sizeof(cwd));
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
```
_Code untuk memulai socket programming dalam server_

```cpp
int main(int argc, char const *argv[])
{
  getcwd(cwd, sizeof(cwd));
  struct sockaddr_in address;
  int sock = 0, valread;
  struct sockaddr_in serv_addr;

  int otentikasi = 0;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Socket creation error \n");
    return -1;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
  {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("\nConnection Failed \n");
    return -1;
  }
```
_Code untuk memulai socket programming dalam client_
<br>
 Untuk melakukan transfer data antara client dan server kami menggunakan potongan program dibawah ini. Untuk mengirim dan menerima pesan dari server dan client kami menggunakan bantuan fungsi `send` untuk melakukan pengiriman buffer data,  `read` untuk menerima dan menyimpan variabel dari data yang dikirimkan, `bzero` untuk me-NULL-kan variabel buffer yang berguna untuk tempat pengiriman ataupun penerimaan data. 

```cpp
    char *message = "user id : ";
    send(new_socket, message, strlen(message), 0);
```
_Code pengiriman data, argumen untuk fungsi `send` ada 4 yaitu `new_socket` sebagai variabel koneksi socket, `message` sebagai pesan yang dikirimkan, `strlen()` sebagai panjangnya data yang dikirimkan , dan 0 sebagai flags default dari fungsi `send`_

```cpp
    int valread;
    char id[1024];
    valread = read(new_socket, id, 1024);
    if (valread < 1)
    {
        printf("eror recv\n");
    }
    printf("%s\n", id);
    strtok(id, "\n");
```
_Code penerimaan data, dimana variabel valread nantinya akan menjadi status keberhasilan penerimaan data, jika fungsi `read` mengembalikan nilai -1 maka akan memunculkan pesan error recv. Fungsi `read` memiliki 3 argument yang pertama `new_socket` berarti file descriptor yang merupakan variabel connect ke server/client, `id` merupakan variabel buffer yang akan menerima pesan yang dikirimkan, dan argumen ketiga yaitu `1024` sebagai panjang file yang mampu ditampung fungsi read._
<br>


## **Kendala**
---

Pada awalnya dari kelompok kami kesulitan untuk mengatasi
<br>


## **Screenshoot Hasil Run Program Tambahan [Tambahan](./client/client.c)**
---
![Foto](./img/soal3/bintang.png)
---

<br>

## Error Handling
---

Source Code tersedia pada : [client.c](./client/client.c)\
Source Code tersedia pada : [dump.c](./client/dump.c)\
Source Code tersedia pada : [database.c](./database/database.c)

## **Analisa Soal**


Hal-hal yang perlu diperhatikan diantaranya :
1. Jika ada command yang tidak sesuai penggunaannya. Maka akan mengeluarkan pesan error tanpa keluar dari program client.

<br>

**Cara pengerjaan**
---


Selain itu kami sangat terbantu dengan template socket programming yang terdapat dalam modul 3 sistem operasi. Kurang lebih dasar pengerjaan kami adalah modul tersebut yang selanjutnya dikembangkan.

```cpp
int main(int argc, char const *argv[])
{
    getcwd(cwd, sizeof(cwd));
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
```
_Code untuk memulai socket programming dalam server_

```cpp
int main(int argc, char const *argv[])
{
  getcwd(cwd, sizeof(cwd));
  struct sockaddr_in address;
  int sock = 0, valread;
  struct sockaddr_in serv_addr;

  int otentikasi = 0;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Socket creation error \n");
    return -1;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
  {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("\nConnection Failed \n");
    return -1;
  }
```
_Code untuk memulai socket programming dalam client_
<br>
 Untuk melakukan transfer data antara client dan server kami menggunakan potongan program dibawah ini. Untuk mengirim dan menerima pesan dari server dan client kami menggunakan bantuan fungsi `send` untuk melakukan pengiriman buffer data,  `read` untuk menerima dan menyimpan variabel dari data yang dikirimkan, `bzero` untuk me-NULL-kan variabel buffer yang berguna untuk tempat pengiriman ataupun penerimaan data. 

```cpp
    char *message = "user id : ";
    send(new_socket, message, strlen(message), 0);
```
_Code pengiriman data, argumen untuk fungsi `send` ada 4 yaitu `new_socket` sebagai variabel koneksi socket, `message` sebagai pesan yang dikirimkan, `strlen()` sebagai panjangnya data yang dikirimkan , dan 0 sebagai flags default dari fungsi `send`_

```cpp
    int valread;
    char id[1024];
    valread = read(new_socket, id, 1024);
    if (valread < 1)
    {
        printf("eror recv\n");
    }
    printf("%s\n", id);
    strtok(id, "\n");
```
_Code penerimaan data, dimana variabel valread nantinya akan menjadi status keberhasilan penerimaan data, jika fungsi `read` mengembalikan nilai -1 maka akan memunculkan pesan error recv. Fungsi `read` memiliki 3 argument yang pertama `new_socket` berarti file descriptor yang merupakan variabel connect ke server/client, `id` merupakan variabel buffer yang akan menerima pesan yang dikirimkan, dan argumen ketiga yaitu `1024` sebagai panjang file yang mampu ditampung fungsi read._
<br>


## **Kendala**
---

Pada awalnya dari kelompok kami kesulitan untuk mengatasi
<br>


## **Screenshoot Hasil Run Program Tambahan [Tambahan](./client/client.c)**
---
![Foto](./img/soal3/bintang.png)
---

<br>
