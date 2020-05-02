# Shift 4 SISOP 2020 - T08
Penyelesaian Soal Shift 4 Sistem Operasi 2020
Kelompok T08
  * I Made Dindra Setyadharma (05311840000008)
  * Muhammad Irsyad Ali (05311840000041)

---

## Table of Contents
* [Pendahuluan](#pendahuluan)
* [Soal 1](#soal-1)
* [Soal 2](#soal-2)
* [Soal 3](#soal-3)
* [Soal 4](#soal-4)

---

## Pendahuluan
Sebelum memulai untuk mengerjakan soal, pertama kami membuat FUSE filesystem yang berjalan seperti file system biasa terlebih dahulu dengan mengimplementasikan *system-call* yang diperlukan untuk soal-soal yang akan dibuat. Maka dari itu, kami mengimplementasikan beberapa *system-call* sebagai berikut.

```c
static const struct fuse_operations _oper = {
	.getattr	= _getattr,
	.access		= _access,
	.readlink	= _readlink,
	.readdir	= _readdir,
	.mkdir		= _mkdir,
	.symlink	= _symlink,
	.unlink		= _unlink,
	.rmdir		= _rmdir,
	.rename		= _rename,
	.link		  = _link,
	.chmod		= _chmod,
	.chown		= _chown,
	.truncate	= _truncate,
	.utimens	= _utimens,
	.open		  = _open,
	.create 	= _create,
	.read		  = _read,
	.write		= _write,
	.statfs		= _statfs,
	.release	= _release,
};
```

Seluruh *system-call* tersebut pada dasarnya hanya memanggil *system-call* linux dengan mengganti `path` yang diinputkan pada *system-call* tersebut menjadi `path` pada mount-pointnya, yaitu pada variable `dirpath`.
* `_getattr()`: untuk mendapatkan **`stat`** dari `path` yang diinputkan.
* `_access()`: untuk mengakses `path` yang diinputkan.
* `_readlink()`: untuk membaca *symbolic link* dari `path`.
* `_mkdir()`: untuk membuat direktori pada `path`.
* `_symlink()`: untuk membuat *symbolic link* pada `path`.
* `_unlink()`: untuk menghapus sebuah file pada `path`.
* `_rmdir()`: untuk menghapus directory pada `path`.
* `_rename()`: untuk me-*rename* dari `path` awal menjadi `path` tujuan.
* `_link()`: untuk membuat *hard-link* pada `path`.
* `_chmod()`: untuk mengubah mode (hak akses) dari `path`.
* `_chown()`: untuk mengubah kepimilikan (user dan group) dari `path`.
* `_truncate()`: untuk melakukan truncate (membesarkan atau mengecilkan size) dari `path`.
* `_utimens()`: untuk mengubah status *time* dari `path`.
* `_open()`: untuk meng-*open* (membuka) `path`.
* `_create()`: untuk membuat `path` berdasarkan mode yang diinputkan.
* `_read()`: untuk membaca isi dari `path`.
* `_write()`: untuk menulis kedalam `path`.
* `_statfs()`: untuk melaukan **`stat`** terhadap file yang diinputkan.
* `_release()`: untuk meng-*release* memori yang dialokasikan untuk system-call `_open()`.

```c
static const char *dirpath = "/home/umum/Documents/SisOpLab/FUSE";
```

Sehingga nantinya ketika filesystem dimount, memiliki root pada dirpath tersebut. Lalu file system tersebut bisa kita edit untuk melakukan operasi yang kita inginkan untuk menjawab keempat soal shift ini. Disini kami mengambil referensi pada [passthrough.c](https://github.com/libfuse/libfuse/blob/master/example/passthrough.c) untuk membuat filesystem tersebut.


## Soal 1
Source Code : [source](https://github.com/DSlite/SoalShiftSISOP20_modul4_T08/blob/master/ssfs.c)

### Deskripsi
Soal meminta kami untuk membuat sebuah metode enkripsi V1, yaitu caesar chipher dengan shift sebesar 10 yang berjalan jika sebuah directory di-**`mkdir`** atau di-**`rename`** dengan awalan `/encv1_` dengan key ```9(ku@AW1[Lmvgax6q`5Y2Ry?+sF!^HKQiBXCUSe&0M.b%rI'7d)o4~VfZ*{#:}ETt$3J-zpc]lnh8,GwP_ND|jO``` yang dimana setiap entry (termasuk sub-entry) pada directory baru tersebut menjadi terenkripsi menggunakan metode enkripsi tersebut (Untuk ekstensi file tidak ikut dienkripsi). Dan jika sebuah directory yang awalnya terenkripsi (berawalan `/encv1_`) di-**`rename`** menjadi directory biasa, maka directory tersebut akan terdekripsi. Setiap penggunaan metode enkripsi pertama akan tercatat kedalam log file.

### Asumsi Soal
Disini, kami membuat metode enkripsi ini seperti pada contoh soal latihan sesi lab 4 untuk meng-reverse seluruh entry menjadi terbalik. Caranya yaitu dengan setiap *system-call* `_readdir()` membaca sebuah directory yang memiliki awalan `/encv1_`, maka nama yang akan diinputkan pada buffer akan di enkripsi terlebih dahulu. Lalu untuk mengatasi entry yang telah terenkripsi tersebut, maka untuk seluruh *system-call* akan mengecek terlebih dahulu apakah path yang diinputkan memiliki directory dengan awalan `/encv1_`, jika ada maka akan dilakukan dekripsi pada path tersebut.

### Pembahasan
Seperti yang dijelaskan pada asumsi soal, maka kami membuat beberapa fungsi yang akan digunakan pada soal ini.
* [`decrypt()`](#fungsi-decrypt): Fungsi untuk melakukan dekripsi dan enkripsi menggunakan metode caesar cipher dengan `key` yang telah dideklarasi.
* [`getDirAndFile()`](#fungsi-getdirandfile): Fungsi untuk mendapatkan `directory` dan `filename` dari `path` yang diinputkan.
* [`changePath()`](#fungsi-changepath): Fungsi untuk mengganti `path` yang diinputkan menjadi **mount-point** yang telah diset. Fungsi ini juga berfungsi untuk mengecek apakah perlu dilakukan dekripsi (directory berawalan `/encv_1`) pada `path` yang diinputkan.
Dan juga untuk melakukan enkripsi, kami melakukannya langsung pada *system-call* [`_readdir()`](#system-call-readdir).

#### Fungsi decrypt

```c
void decrypt(char *path, int isEncrypt) {
  char *cursor = path;
  while (cursor-path < strlen(path)) {
    char *ptr = strchr(key, *cursor);
    if (ptr != NULL) {
      int index = (ptr-key+strlen(key)-10)%strlen(key);
      if (isEncrypt) index = (ptr-key+strlen(key)+10)%strlen(key);
      *cursor = *(key+index);
    }
    cursor++;
  }
}
```

Fungsi `decrypt()` memiliki dua arguments, yaitu `path` sebagai alamat yang akan didekripsi atau dienkripsi dan argument `isEncrypt` untuk deklarasi ingin melakukan enkripsi atau dekripsi. Lalu deklarasi variable `*cursor` untuk menunjuk pada karakter pertama `path`. `while()` disini akan melakukan iterasi setiap karakter yang ada di `path` selama variable `*cursor` masih menunjuk karakter pada `path` (alamat yang ditunjuk `*cursor` - alamat yang ditunjuk `path` < panjang string). Tiap iterasi akan membuat `*ptr` untuk menunjuk karakter pada `key` yang sesuai dengan yang ditunuk pada `*cursor` menggunakan fungsi `strchr()`. Untuk dekripsi, akan dicari `index` dari dekripsi karakter yang ditunjuk `key` dengan mengurangi 10. maka, untuk mendapatkan `index` awal, alamat `*ptr` dan alamat `key` dikurangi lalu ditambah panjang `key` (agar nilainya tidak negatif) lalu dikurangi 10, dan terakhir akan dimodulo dengan panjang `key`. Maka `index` akan menyimpan index dari karakter hasil dekripsi dari karakter yang ditunjuk pada `*ptr`. Untuk enkripsi, `index` diset ulang agar melakukan enkripsi (+10). Lalu posisi `*cursor` akan di-increment ke karakter berikutnya.

#### Fungsi getDirAndfile

```c
void getDirAndFile(char *dir, char *file, char *path) {
  char buff[1000];
  memset(dir, 0, 1000);
  memset(file, 0, 1000);
  strcpy(buff, path);
  char *token = strtok(buff, "/");
  while(token != NULL) {
    sprintf(file, "%s", token);
    token = strtok(NULL, "/");
    if (token != NULL) {
      sprintf(dir, "%s/%s", dir, file);
    }
  }
}
```

Fungsi ini akan membagi `path` yang diinputkan menjadi `dir` dan `file` berdasarkan lokasi **`/`** terakhir. Pertama deklarasi variable `buff` untuk menyimpan `path` agar tidak mengubah isi dari `path` ketika dilakukan `strtok()`. Lalu `dir` dan `file` akan di `memset()` agar isinya menjadi kosong. Lalu `path` akan di-`strcpy()` kedalam `buff`. Lalu akan membuat `*token` dengan `strtok()` pada `buff` dengan delimiternya `"/"`. Lalu akan diiterasi untuk masing-masing token. Tiap iterasi akan meng-*copy* isi `token` kedalam variable `file` menggunakan fungsi `sprintf()`, lalu token akan diiterasi ke token selanjutnya. Di akhir akan dicek apakah token tersebut belum token terakhir, jika belum, maka akan mengupdate variable `dir` agar menjadi `dir/file` menggunakan fungsi `sprintf()`.

#### Fungsi changePath

```c
void changePath(char *fpath, const char *path, int isWriteOper, int isFileAsked) {
  char *ptr = strstr(path, "/encv1_");
  int state = 0;
  if (ptr != NULL) {
    if (strstr(ptr+1, "/") != NULL) state = 1;
  }
  char fixPath[1000];
  memset(fixPath, 0, sizeof(fixPath));
  if (ptr != NULL && state) {
    ptr = strstr(ptr+1, "/");
    char pathEncvDirBuff[1000];
    char pathEncryptedBuff[1000];
    strcpy(pathEncryptedBuff, ptr);
    strncpy(pathEncvDirBuff, path, ptr-path);
    if (isWriteOper) {
      char pathFileBuff[1000];
      char pathDirBuff[1000];
      getDirAndFile(pathDirBuff, pathFileBuff, pathEncryptedBuff);
      decrypt(pathDirBuff, 0);
      sprintf(fixPath, "%s%s/%s", pathEncvDirBuff, pathDirBuff, pathFileBuff);
    } else if (isFileAsked) {
      char pathFileBuff[1000];
      char pathDirBuff[1000];
      char pathExtBuff[1000];
      getDirAndFile(pathDirBuff, pathFileBuff, pathEncryptedBuff);
      char *whereIsExtension = strrchr(pathFileBuff, '.');
      if (whereIsExtension-pathFileBuff < 1) {
        decrypt(pathDirBuff, 0);
        decrypt(pathFileBuff, 0);
        sprintf(fixPath, "%s%s/%s", pathEncvDirBuff, pathDirBuff, pathFileBuff);
      } else {
        char pathJustFileBuff[1000];
        memset(pathJustFileBuff, 0, sizeof(pathJustFileBuff));
        strcpy(pathExtBuff, whereIsExtension);
        snprintf(pathJustFileBuff, whereIsExtension-pathFileBuff+1, "%s", pathFileBuff);
        decrypt(pathDirBuff, 0);
        decrypt(pathJustFileBuff, 0);
        sprintf(fixPath, "%s%s/%s%s", pathEncvDirBuff, pathDirBuff, pathJustFileBuff, pathExtBuff);
      }
    } else {
      decrypt(pathEncryptedBuff, 0);
      sprintf(fixPath, "%s%s", pathEncvDirBuff, pathEncryptedBuff);
    }
  } else {
    strcpy(fixPath, path);
  }
  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, fixPath);
  }
}
```

```c
void changePath(char *fpath, const char *path, int isWriteOper, int isFileAsked) {
  char *ptr = strstr(path, "/encv1_");
  int state = 0;
  if (ptr != NULL) {
    if (strstr(ptr+1, "/") != NULL) state = 1;
  }

  ...

}
```

Pertama tama kami mendefinisikan fungsi ini dengan 4 arguments. Argument pertama `fpath` yang digunakan untuk buffer hasil perubahan `path` menjadi path yang baru. Argumen kedua `path` yang akan diinput oleh masing-masing *system-call* untuk mengubah pathnya sesuai **mount-point** atau **metode enkripsi 1**. Argument ketiga `isWriteOper` untuk mendefinisikan apakah *system-call* yang memanggil fungsi ini memiliki operasi write (karena ketika kita ingin membuat file pada directory yang terenkripsi dengan **`/encv1_`**, maka path untuk filename yang di write tidak didekripsi maupun dienkripsi). Argument keempat `isFileAsked` untuk mendefinisikan apakah *system-call* yang memanggil fungsi ini ingin melakukan dekripsi pada sebuah file atau sebuah directory (pada directory, dekripsi akan dilakukan langsung sementara pada file harus mengecek ekstensinya terlebih dahulu).\

Variable `*ptr` digunakan untuk menunjuk posisi `/encv1_` pada `path` menggunakan fungsi `strstr()` (Contoh: `/contoh/encv1_dir1/test` -> `/encv1_dir1/test`). Lalu akan deklarasi variable `state` untuk memastikan apakah `path` yang diinputkan berisi directory didalamnya atau hanya `/encv1_` (Contoh: `/contoh/encv1_dir1` tidak akan didekripsi, sementara `/contoh/encv1_dir1/test` akan didekripsi). Lalu akan dicek apakah string setelah karakter pertama dari `ptr` memiliki karakter `/`. Jika iya, maka state akan diset menjadi `1`.

```c
  char fixPath[1000];
  memset(fixPath, 0, sizeof(fixPath));
  if (ptr != NULL && state) {
    ptr = strstr(ptr+1, "/");
    char pathEncvDirBuff[1000];
    char pathEncryptedBuff[1000];
    strcpy(pathEncryptedBuff, ptr);
    strncpy(pathEncvDirBuff, path, ptr-path);

    ...

  }
```

Pendefinisian buffer `fixpath[]` yang digunakan untuk menyimpan proccessed path dari tiap kondisi yang berbeda. `if()` disini akan berjalan ketika terdapat substring `/encv1_` pada `path` dengan kondisi `state` **true**. lalu `ptr` akan diset menjadi path setelah `/encv1_` (Contoh: `/encv1_dir1/test` akan menjadi `/test` pada `ptr`). Lalu akan didefinisikan `pathEncryptedBuff` untuk menyimpan path `*ptr` yang telah dicari menggunakan fungsi `strcpy()`, dan juga didefinisikan `pathEncvDirBuff` untuk menyimpan `path` yang tidak didekripsi menggunakan fungsi `strncpy()` dari karakter pertama `path` sampai panjang `ptr-path` (Contoh: `/dir/encv1_dir1/test` akan disimpan menjadi `/dir/encv1_dir1` pada `pathEncvDirBuff` dan `/test` pada `pathEncryptedBuff`).

```c
    if (isWriteOper) {
      char pathFileBuff[1000];
      char pathDirBuff[1000];
      getDirAndFile(pathDirBuff, pathFileBuff, pathEncryptedBuff);
      decrypt(pathDirBuff, 0);
      sprintf(fixPath, "%s%s/%s", pathEncvDirBuff, pathDirBuff, pathFileBuff);
    } else if ...
```

Dalam kondisi **`isWriteOper`**, yang didekripsi hanya path **directory**nya saja. Sehingga `pathEncryptedBuff` perlu dipisah menggunakan fungsi `getDirAndFile()` dan disimpan pada `pathDirBuff` dan `pathFileBuff`. Lalu akan menjalankan fungsi `decrypt()` pada `pathDirBuff` saja. Setelah itu, seluruh path akan dijadikan satu kedalam buffer `fixPath` dengan urutan `pathEncvDirBuff` (path tidak terenkripsi), `pathDirBuff` (path yang dienkripsi), dan terakhir `pathFileBuff` (nama file).

```c
    } else if (isFileAsked) {
      char pathFileBuff[1000];
      char pathDirBuff[1000];
      char pathExtBuff[1000];
      getDirAndFile(pathDirBuff, pathFileBuff, pathEncryptedBuff);
      char *whereIsExtension = strrchr(pathFileBuff, '.');
      if (whereIsExtension-pathFileBuff < 1) {
        decrypt(pathDirBuff, 0);
        decrypt(pathFileBuff, 0);
        sprintf(fixPath, "%s%s/%s", pathEncvDirBuff, pathDirBuff, pathFileBuff);
      } else {
        char pathJustFileBuff[1000];
        memset(pathJustFileBuff, 0, sizeof(pathJustFileBuff));
        strcpy(pathExtBuff, whereIsExtension);
        snprintf(pathJustFileBuff, whereIsExtension-pathFileBuff+1, "%s", pathFileBuff);
        decrypt(pathDirBuff, 0);
        decrypt(pathJustFileBuff, 0);
        sprintf(fixPath, "%s%s/%s%s", pathEncvDirBuff, pathDirBuff, pathJustFileBuff, pathExtBuff);
      }
    } else ...
```

Bagian ini berjalan untuk kondisi dimana bukan `isWriteOper` dan yang diminta merupakan `isFileAsked`. Lalu deklarasi awal `pathFileBuff[]`, `pathDirBuff[]`, dan `pathExtBuff[]` untuk menyimpan `file`, `directory`, dan `ekstensi` dari `pathEncryptedBuff`. Maka pertama, path `directory` dan path `file` dipisah dahulu menggunakan fungsi `getDirAndFile()`. Lalu untuk mencari ekstensi, akan dideklarasi pointer `*whereIsExtension` untuk menunjuk karakter '.' terakhir pada `pathFileBuff` menggunakan fungsi `strrchr()`. Lalu dicek apakah karakter '.' terakhir ini merupakan karakter pertama (kalau karakter pertama berarti `pathFileBuff` tidak memiliki ektensi seperti file dengan nama `.contoh`).\

Jika iya, maka `pathDirBuff` dan `pathFileBuff` akan didekripsi menggunakan fungsi `decrypt()` dan disatukan kembali pada variable `fixPath` sesuai formatnya. Jika ternyata terdapat ekstensi, maka akan dideklarasi `pathJustFileBuff` untuk menyimpan nama filenya saja menggunakan `snprintf` dan mengisi value `pathExtBuff` menjadi ekstensinya saja. Lalu `pathDirBuff` dan `pathJustFileBuff` akan didekripsi menggunakan fungsi `decrypt()` dan disatukan kembali pada variable `fixPath` sesuai formatnya.

```c
    } else {
      decrypt(pathEncryptedBuff, 0);
      sprintf(fixPath, "%s%s", pathEncvDirBuff, pathEncryptedBuff);
    }
  } else {
    strcpy(fixPath, path);
  }
  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, fixPath);
  }
}
```

`else` pertama disini untuk kondisi bukan `isWriteOper` dan yang diminta bukan merupakan `isFileAsked` atau *directory* sehingga isi dalam buffer `pathEncryptedBuff` akan langsung didekripsi dan disimpan kedalam `fixPath`.\

`else` kedua adalah untuk kondisi `state == 0` atau tidak ada karakter `encv1_` maka isi dari `fixPath` akan sama dengan `path` karena tidak ada metode enkripsi dan dekripsi yang akan dijalankan.\

selanjutnya adalah mengubah `fixPath` menjadi `fpath` dengan mengisi **`mount-point`** yang telah dideklarasi pada variable `dirpath`. Caranya sama seperti soal latihan, yaitu jika `path` yang diinputkan merupakan root `"/"` maka dirpath langsung dicopy kedalam `fpath`, sementara bukan, maka akan diformat agar fixPath menuju pada **`mount-point`** yang terletak pada `dirpath`.

```c
static const char *dirpath = "/home/umum/Documents/SisOpLab/FUSE";
```

#### System-call readdir

```c
static int _readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {

  ...

  DIR *dp;
  struct dirent *de;

  ...

  dp = opendir(fpath);

  ...

  while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
    if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
    if (strstr(path, "/encv1_") != NULL) {
      char encryptThis[1000];
      strcpy(encryptThis, de->d_name);
      if (de->d_type == DT_REG) {
        char *whereIsExtension = strrchr(encryptThis, '.');
        if (whereIsExtension-encryptThis < 1) {
          decrypt(encryptThis, 1);
        } else {
          char pathFileBuff[1000];
          char pathExtBuff[1000];
          strcpy(pathExtBuff, whereIsExtension);
          snprintf(pathFileBuff, whereIsExtension-encryptThis+1, "%s", encryptThis);
          decrypt(pathFileBuff, 1);
          memset(encryptThis, 0, sizeof(encryptThis));
          sprintf(encryptThis, "%s%s", pathFileBuff, pathExtBuff);
        }
      } else if (de->d_type == DT_DIR) {
        decrypt(encryptThis, 1);
      }

  		if (filler(buf, encryptThis, &st, 0)) break;
    } else {
      if (filler(buf, de->d_name, &st, 0)) break;
    }
	}

  ...

}
```

*System-call* `_mkdir()` ini digunakan untuk melakukan enkripsi pada entry yang berada pada directory `/encv1_`. Pada fungsi diatas, jika pada path terdapat `/encv1_`, maka akan dilakukan enkripsi.\

Pertama, akan mendeklarasi variable `encryptThis` dengan mengambil nama dari entrynya (`de->d_name`). Lalu jika entry tersebut merupakan `DT_REG` (file reguler) maka akan dilakukan enkripsi file (ekstensi tidak terenkripsi). Pertama akan dicari lokasi ekstensi menggunakan pointer `*whereIsExtension` untuk menunjuk lokasinya dan fungsi `strrchr()`. Lalu dicek apakah file tersebut memiliki ekstensi atau tidak. Jika tidak, maka `encryptThis` akan langsu di enkripsi dengan fungsi `decrypt()` dengan argument `isEncrypt = 1`. Jika terdapat ekstensi, maka antara nama file dan extensinya dipisah kedalam `pathFileBuff` dan `pathExtBuff` menggunakan cara yang sama pada fungsi `changePath()`. lalu `pathFileBuff` akan dienkripsi menggunakan fungsi `decrypt()`, nama file dan ekstensinya akan disatukan kembali dan disimpan dalam variable `encryptThis`.\

Jika ternyata tipe dari entry tersebut `DT_DIR`, maka `encryptThis` akan langsung dienkripsi. Setelah proses enkripsi selesai untuk entry tersebut, maka akan dimasukkan kedalam `buf` menggunakan fungsi `filler()`.\

Jika `path` yang dimasukkan tidak memiliki `/encv1_` didalamnya, maka nama entry (`de->d_name`) akan langsung dimasukkan kedalam buf menggunakan fungsi `filler()`.

#### Contoh Implementasi System-call *`write operation`* Untuk Soal 1

```c
static int _mkdir(const char *path, mode_t mode)
{
	char fpath[1000];
	changePath(fpath, path, 1, 0);

  char *ptr = strrchr(path, '/');
  char *filePtr = strstr(ptr, "/encv1_");
  if (filePtr != NULL) {
    if (filePtr - ptr == 0) {
      const char *desc[] = {path};
      logFile("SPECIAL", "ENCV1", 0, 1, desc);
    }
  }

	int res;

	res = mkdir(fpath, mode);

  ...

	if (res == -1) return -errno;

	return 0;
}
```

Pada contoh diatas, merupakan *system-call* `_mkdir()`. Pada *system-call* tersebut, akan dilakukan `changePath()` terhadap `path` untuk mendapatkan `fpath`, arguments `isWriteOper = 1` karena `mkdir` merupakan operasi *write* (membuat). Lalu karena setiap pembuatan directory `/encv1_`, maka harus di-log. Untuk melakukan logging, kami menggunakan fungsi `logFile` yang akan dijelaskan pada [soal 4](#soal-4). setelah itu `mkdir` akan dijalankan seperti biasa dan resultnya akan direturn. logging tersebut juga dilakukan pada system-call `_rename()` dengan metode yang sama.

#### Contoh Implementasi System-call *`read operation`* Untuk Soal 1

```c
static int _getattr(const char *path, struct stat *stbuf) {

	char fpath[1000];
  changePath(fpath, path, 0, 1);
  if (access(fpath, F_OK) == -1) {
    memset(fpath, 0, sizeof(fpath));
    changePath(fpath, path, 0, 0);
  }

	int res;

	res = lstat(fpath, stbuf);

  ...

	if (res == -1) return -errno;


	return 0;
}
```

Pada contoh diatas, merupakan *system-call* `_getattr()`. Pada *system-call* tersebut, akan dilakukan `changePath()` terhadap `path` untuk mendapatkan `fpath`. Karena ketika `path` yang dimasukkan kita tidak tahu apakah file tersebut merupakan file biasa atau merupakan directory maka kita akan mengubah terlebih dahulu dengan asumsi bahwa `path` tersebut merupakan file, sehingga argument `isFileAsked = 1`. Lalu akan dicoba di `access` apakah bisa, jika tidak bisa diakses, maka akan dilakukan changePath kembali dengan argument `isFileAsked = 0` (directory). Lalu proses `getattr` akan berlangsung pada `fpath` tersebut.

### Kesulitan
Debuggingnya, karena harus langsung pada file system.

### ScreenShot
![Output](https://user-images.githubusercontent.com/17781660/80861749-aa67c100-8c9a-11ea-84aa-9a5c1282155f.png)
![Output](https://user-images.githubusercontent.com/17781660/80861752-ac318480-8c9a-11ea-874b-73568a72ac4e.png)
![Output](https://user-images.githubusercontent.com/17781660/80861753-ad62b180-8c9a-11ea-8819-f01040eb58e3.png)
![Output](https://user-images.githubusercontent.com/17781660/80861754-adfb4800-8c9a-11ea-8cad-a5f6116c8f17.png)

## Soal 2
Source Code : [source](https://github.com/DSlite/SoalShiftSISOP20_modul4_T08/blob/master/ssfs.c)

### Deskripsi
Soal meminta kami untuk membuat sebuah metode enkripsi V2 yang berjalan jika sebuah directory di-**`mkdir`** atau di-**`rename`** dengan awalan `/encv2_` Yang dimana setiap directory yang sudah terenkripsi akan terdekripsi jika namanya di `rename` menjadi tanpa awalan `/encv2_`. Setiap file yang ada pada direktori asli akan dijadikan bagian-bagian kecil sebesar 1024 bytes (1 kB) dan kembali normal jika directory tersebut terdekripsi. Sebagai contoh, file File_Contoh.txt berukuran 5 kB pada direktori asli akan menjadi 5 file kecil yakni: File_Contoh.txt.000, File_Contoh.txt.001, File_Contoh.txt.002, File_Contoh.txt.003, dan File_Contoh.txt.004. Dan berlaku untuk setiap isi dari subfolder. Setiap penggunaan metode enkripsi kedua akan tercatat kedalam log file.

### Asumsi Soal
Disini, untuk membuat metode enkripsi tersebut kami hanya mengimplementasikan pada *system-call* `_rename()` saja. Jadi nantinya ketika sebuah directory berubah nama menjadi berawalan `/encv2_`, maka akan menjalankan fungsi untuk mempartisi tiap file yang ada didalam directory dan sub-directory tersebut. Jika directory berubah nama dari berawalan `/encv2_`, maka akan menjalankan fungsi dekripsinya untuk menggabungkan partisi tiap file yang ada didalam directory dan sub-directory tersebut. Untuk Logging akan dilakukan ketika `_mkdir()` dan `_rename()` dengan cara yang sama dengan nomor 1.

### Pembahasan
Seperti yang dijelaskan pada asumsi soal, maka kami membuat beberapa fungsi yang akan digunakan pada soal ini.
* [`splitter()`](#fungsi-splitter): Fungsi untuk melakukan split (partisi) secara rekursif setiap entry **file** pada `path` yang diinputkan.
* [`unsplitter()`](#fungsi-unsplitter): Fungsi untuk melakukan unsplit (join) secara rekursif setiap entry **file** pada `path` yang diinputkan.
Dan untuk melakukan proses enkripsi dan dekripsinya akan berada pada *system-call* [`_rename()`](#system-call-rename).

#### Fungsi splitter

Berfungsi untuk melakukan split (partisi) pada tiap file yang ada di dalam directory secara rekursif jika ada sub directory

```c
void splitter(char *path) {
  DIR *dp;
  struct dirent *de;
  dp = opendir(path);
  while ((de = readdir(dp)) != NULL) {
    if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
    char newPath[1000];
    sprintf(newPath, "%s/%s", path, de->d_name);
    if (de->d_type == DT_DIR) {
      splitter(newPath);
    }
    if (de->d_type == DT_REG) {
      FILE *pathFilePointer = fopen(newPath, "rb");
      fseek(pathFilePointer, 0, SEEK_END);
      long pathFileSize = ftell(pathFilePointer);
      rewind(pathFilePointer);
      struct stat st;
      lstat(newPath, &st);
      int index = 0;
      while(pathFileSize > 0) {
        char newFileSplit[1000];
        char buff[1024];
        sprintf(newFileSplit, "%s.%03d", newPath, index++);
        close(creat(newFileSplit, st.st_mode));
        int size;
        if (pathFileSize >= 1024) {
          size = 1024;
          pathFileSize-=1024;
        } else {
          size = pathFileSize;
          pathFileSize = 0;
        }
        memset(buff, 0, sizeof(buff));
        fread(buff, 1, size, pathFilePointer);

        FILE *pathFileOut = fopen(newFileSplit, "wb");
        fwrite(buff, 1, size, pathFileOut);
        fclose(pathFileOut);
      }
      fclose(pathFilePointer);
      unlink(newPath);
    }
  }
}
```

```c
void splitter(char *path) {
  DIR *dp;
  struct dirent *de;
  dp = opendir(path);
  while ((de = readdir(dp)) != NULL) {
    if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
    char newPath[1000];
    sprintf(newPath, "%s/%s", path, de->d_name);

    ...

  }
}
```

Pertama kami akan membuka `path` yang di berikan dengan `opendir()` kedalam variable `dp`, selanjutnya `while()` loop akan berjalan untuk masing masing entry dari `dp` sampai habis, kami menggunakan exception untuk karakter `.` dan `..`, akan langsung di `continue`. Selanjutnya kami membuat full path yang baru kepada buffer `newPath` yang berisi `path` dan `de->name` dengan menggunakan fungsi `sprintf()`.

```c
    if (de->d_type == DT_DIR) {
      splitter(newPath);
    }
    if (de->d_type == DT_REG) {
      FILE *pathFilePointer = fopen(newPath, "rb");
      fseek(pathFilePointer, 0, SEEK_END);
      long pathFileSize = ftell(pathFilePointer);
      rewind(pathFilePointer);
      struct stat st;
      lstat(newPath, &st);
      int index = 0;

      ...

    }
```

Setelah itu, jika entry tersebut merupakan `DT_DIR` (directory), maka akan menjalankan `splitter()` pada `newPath` yang telah dideklarasi (rekursif).\

Kemudian, jika entry tersebut merupakan `DT_REG` (file biasa) akan dilakukan splitting (partisi). Untuk setiap file
reguler maka akan dibuat path ke file pointernya dengan menggunakan `fopen()` dengan mode `"rb"` agar melakukan *read* dalam bentuk *bytes*. Kemudian kami menggunakan fungsi `fseek()` menempatkan posisi pointer `pathFilePointer` ke akhir file dan fungsi `ftell()` untuk mencari tahu size dari file yang ditunjuk `pathFilePointer`. Kemudian pointer akan dikembalikan ke awal file menggunakan fungsi `rewind()`, selanjutnya kami menggunakan `lstat()` agar access modifiernya
sama dengan kondisi awalnya. Lalu deklarasi variable `index` untuk mengetahui nomor part yang sedang diwrite.

```c
      while(pathFileSize > 0) {
        char newFileSplit[1000];
        char buff[1024];
        sprintf(newFileSplit, "%s.%03d", newPath, index++);
        close(creat(newFileSplit, st.st_mode));
        int size;
        if (pathFileSize >= 1024) {
          size = 1024;
          pathFileSize-=1024;
        } else {
          size = pathFileSize;
          pathFileSize = 0;
        }

        ...

      }
```

Bagian ini adalah `while()` loop yang berjalan sampai `pathFilePointer` terbaca semua. Tiap loop akan mendeklarasikan `newFileSplit[]` untuk menyimpan `path` dari partisinya dan deklarasi variable `buff[]` untuk menyimpan hasil read dari file `newPath` dan memasukkannya kedalam `newFileSplit`. Kemudian `path` dari file baru ini akan diformat menggunakan `sprintf` berdasarkan `index`-nya. `index` nanti akan di increment setelah `path` `newFileSplit` terbentuk. Lalu akan membuat file partisi tersebut menggunakan kombinasi *system-call* `close()` dan `creat()` dengan access modifier yang telah didapatkan pada variable `st`.\

Selanjutnya, kami akan mencari `size` yang akan dibaca dari pointer `pathFilePointer`. Jika `pathFileSize >= 1024`, maka `size` yang akan dibaca 1024 bytes dan `pathFileSize` akan dikurangi 1024. Jika `pathFileSize` sudah kurang dari 1024, maka `size = pathFileSize` dan `pathFileSize` akan diset menjadi 0.

```c
        memset(buff, 0, sizeof(buff));
        fread(buff, 1, size, pathFilePointer);

        FILE *pathFileOut = fopen(newFileSplit, "wb");
        fwrite(buff, 1, size, pathFileOut);
        fclose(pathFileOut);
      }
      fclose(pathFilePointer);
      unlink(newPath);
    }
  }
}
```

Pada bagian ini, `pathFilePointer` akan di dibaca menggunakan fungsi `fread()` sebesar `size` kedalam `buff` dengan membaca `1` byte. Setelah itu, akan dideklarasi `pathFileOut` untuk *writing* partisinya. Lalu menjalankan fungsi `fwrite()` untuk *writing* variable `buff` sebesar `size` dengan menulis per byte kedalam `pathFileOut`. Lalu `pathFileOut` di `fclose()`. Setelah semua isi dari `pathFilePointer` terbaca, maka `pathFilePointer` akan di `fclose()` dan `newPath` akan di `unlink()` (di-remove).

#### Fungsi Unsplitter

Berfungsi untuk melakukan unsplit (join) pada tiap partisi yang ada di dalam directory secara rekursif jika ada sub directory.

```c
void unsplitter(char *path) {
  DIR *dp;
  struct dirent *de;
  dp = opendir(path);
  while ((de = readdir(dp)) != NULL) {
    if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
    if (de->d_type == DT_DIR) {
      char newPath[1000];
      sprintf(newPath, "%s/%s", path, de->d_name);
      unsplitter(newPath);
    }
    if (de->d_type == DT_REG) {
      char *ptr = strrchr(de->d_name, '.');
      if (strcmp(ptr, ".000") != 0) continue;
      char pathFileName[1000];
      char pathToFile[1000];
      snprintf(pathFileName, ptr-(de->d_name)+1, "%s", de->d_name);
      sprintf(pathToFile, "%s/%s", path, pathFileName);

      char temp[1000];
      sprintf(temp, "%s.000", pathToFile);
      struct stat st;
      lstat(temp, &st);
      close(creat(pathToFile, st.st_mode));

      int index = 0;
      FILE *pathFilePointer = fopen(pathToFile, "wb");
      while (1) {
        char partName[1000];
        sprintf(partName, "%s.%03d", pathToFile, index++);
        if (access(partName, F_OK) == -1) break;
        FILE *pathToPart = fopen(partName, "rb");
        fseek(pathToPart, 0, SEEK_END);
        long pathFileSize = ftell(pathToPart);
        rewind(pathToPart);
        char buff[1024];
        fread(buff, 1, pathFileSize, pathToPart);
        fwrite(buff, 1, pathFileSize, pathFilePointer);
        fclose(pathToPart);
        unlink(partName);
      }
      fclose(pathFilePointer);
    }
  }
}
```

```c
void unsplitter(char *path) {
  DIR *dp;
  struct dirent *de;
  dp = opendir(path);

  ...

}
```

Pertama kami akan membuka path yang di berikan dengan `opendir()` kedalam variable `dp`.

```c
  while ((de = readdir(dp)) != NULL) {
    if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
    if (de->d_type == DT_DIR) {
      char newPath[1000];
      sprintf(newPath, "%s/%s", path, de->d_name);
      unsplitter(newPath);
    }
    if (de->d_type == DT_REG) {
      char *ptr = strrchr(de->d_name, '.');
      if (strcmp(ptr, ".000") != 0) continue;
      char pathFileName[1000];
      char pathToFile[1000];
      snprintf(pathFileName, ptr-(de->d_name)+1, "%s", de->d_name);
      sprintf(pathToFile, "%s/%s", path, pathFileName);

      ...

    }

    ...

  }
```

Pada bagian ini `while()` loop akan berjalan untuk masing masing entry sampai habis, kami menggunakan exception untuk string `.` dan `..`, akan langsung di `continue`.\

`if()` pertama akan melakukan pengecekan entry data, jika typenya merupakan directory maka akan dibuat full path
yang baru kepada buffer `newPath` yang berisi `path` dan `de->name` dengan menggunakan fungsi `sprintf()`, dan menjalankan fungsi `unsplitter` kepada `newPath`, dalam kata lain system call ini akan berjalan rekursif untuk setiap
sub-directory.\

`if()` kedua juga merupakan pengecekan entry data, jika typenya merupakan file regular maka akan di-set sebuah pointer yang menunjuk kepada file yang memliki `.000` di bagian belakangnya menggunakan `if (strcmp(ptr, ".000") != 0) continue` karena proses unsplitting ini dumulai dari `file.000`, selain dari kondisi tersebut maka akan di `continue`, lalu kami mendefinisikan dua buffer `pathFileName[1000]` untuk menyimpan nama filenya saja dan `pathToFile[1000]` untuk path utuh dari file yang akan di create.\

Kemudian `pathFileName` akan diisi dengan hanya nama dari filename, tanpa menggunakan ekstensi (.000,.001,...) menggunakan `snprintf(pathFileName, ptr-(de->d_name)+1, "%s", de->d_name)` argumen kedua mengindikasikan banyak karakter yang di `snpritf()` yaitu mulai dari awal hingga nama file saja.\

Selanjutnya `path` sebagai path awal dan `pathFileName` akan disatukan dimasukan ke `pathTofile` menggunakan `sprintf(pathToFile, "%s/%s", path, pathFileName)`.

```c
      char temp[1000];
      sprintf(temp, "%s.000", pathToFile);
      struct stat st;
      lstat(temp, &st);
      close(creat(pathToFile, st.st_mode));
      int index = 0;
      FILE *pathFilePointer = fopen(pathToFile, "wb");
```

Pada bagian ini kami mendefinisikan buffer `temp[]` untuk menyimpan stat dari `.000` menggunakan `sprintf(temp, "%s.000", pathToFile)`. Kemudian `pathTofile` akan di *create* dengan mode yang sudah di dapatkan mengguanakan `close(creat(pathToFile, st.st_mode))` yang kemudian akan di tutup kembali. Selanjutnya kami membuat variable `index` untuk penanda part ke berapa dan kami membuat pointer `pathFilePinter` yang menunjuk kepada isi dari `pathToFile` menggunakan `FILE *pathFilePointer = fopen(pathToFile, "wb")`.

```c
      while (1) {
        char partName[1000];
        sprintf(partName, "%s.%03d", pathToFile, index++);
        if (access(partName, F_OK) == -1) break;
        FILE *pathToPart = fopen(partName, "rb");
        fseek(pathToPart, 0, SEEK_END);
        long pathFileSize = ftell(pathToPart);
        rewind(pathToPart);

        char buff[1024];
        fread(buff, 1, pathFileSize, pathToPart);
        fwrite(buff, 1, pathFileSize, pathFilePointer);
        fclose(pathToPart);
        unlink(partName);
      }
      fclose(pathFilePointer);
    }
  }
}
```

Pada bagian ini adalah infinite looping menggunakan `while(1)` karena jumlah part dari `pathToFile` masih belum di ketahui. Sehingga [ertama kami akan membuat part name untuk isi dari `pathToFile` dengan menambahkan `%0.3d` berdasarkan `index` yang di increment menggunakan `sprintf(partName, "%s.%03d", pathToFile, index++)`.\

Kemudian `if()` disini berfungsi untuk pengecekan part, menggunakan `if (access(partName, F_OK) == -1) break` yang berarti loop akan di `break` jika part sudah habis. Lalu kami membuat pointer yang menunjuk pada isi dari `partName` yang sudah dibuat menggunakan `FILE *pathToPart = fopen(partName, "rb")`. Selanjutnya kami menggunakan `fseek()` dan `ftell()` untuk mencari `pathFileSize` dari `pathToPart` dan akan di `rewind()` agar pointer kembali ke awal.\

Lalu kami melakukan read pada `pathToPart` sebesar `pathFileSize` kedalam `buff` menggunakan `fread(buff, 1, pathFileSize, pathToPart)` yang kemudian akan di write kedalam `pathFilePointer` menggunakan `fwrite(buff, 1, pathFileSize, pathFilePointer`. Selanjutnya `pathToPart` akan di `fclose()` dan `partName` akan di `unlink()` agar file partisi tersebut terhapus. Terakhir setelah loop selesai pointer `pathFilePointer` akan di `fclose()`.

#### System-call rename

```c
static int _rename(const char *from, const char *to) {
	char ffrom[1000];
	char fto[1000];
	changePath(ffrom, from, 0, 1);
	changePath(fto, to, 0, 1);
  if (access(ffrom, F_OK) == -1) {
    memset(ffrom, 0, sizeof(ffrom));
    changePath(ffrom, from, 0, 0);
  }
  if (access(fto, F_OK) == -1) {
    memset(fto, 0, sizeof(fto));
    changePath(fto, to, 0, 0);
  }

  char *toPtr = strrchr(to, '/');
  char *fromPtr = strrchr(from ,'/');
  char *toStartPtr = strstr(toPtr, "/encv2_");
  char *fromStartPtr = strstr(fromPtr, "/encv2_");
  if (toStartPtr != NULL) {
    if (toStartPtr - toPtr == 0) {
      DIR *d = opendir(ffrom)
      if (d) {
        closedir(d);
        splitter(ffrom);
        const char *desc[] = {fto};
        logFile("SPECIAL", "ENCV2", 0, 1, desc);
      }
    }
  }
  if (fromStartPtr != NULL) {
    if (fromStartPtr - fromPtr == 0) {
      DIR *d = opendir(ffrom);
      if (d) {
        closedir(d);
        unsplitter(ffrom);
      }
    }
  }
  toStartPtr = strstr(toPtr, "/encv1_");
  if (toStartPtr != NULL) {
    if (toStartPtr - toPtr == 0) {
      const char *desc[] = {fto};
      logFile("SPECIAL", "ENCV1", 0, 1, desc);
    }
  }

	int res;

	res = rename(ffrom, fto);

  const char *desc[] = {from, to};
  logFile("INFO", "RENAME", res, 2, desc);

	if (res == -1) return -errno;

	return 0;
}
```

Seperti yang telah dijelaskan pada asumsi soal, kami akan mengecek untuk masing-masing `path`. Jika `from` memiliki `/encv2_`, maka `ffrom` akan dilakukan fungsi `unsplitter()`. Jika `to` memiliki `/encv2_`, maka `ffrom` akan dijalankan fungsi `splitter()` dan juga akan dilakukan logging menggunakan fungsi `logFile`.

### Kesulitan
Membuat write operation pada directory yang telah terenkripsi.

### ScreenShot
![Output](https://user-images.githubusercontent.com/17781660/80863347-db012800-8ca5-11ea-91d4-b99b15d89e21.png)
![Output](https://user-images.githubusercontent.com/17781660/80863350-dccaeb80-8ca5-11ea-86cf-c56b5da8056a.png)
![Output](https://user-images.githubusercontent.com/17781660/80863353-dfc5dc00-8ca5-11ea-9098-8ed0c4b520e0.png)
![Output](https://user-images.githubusercontent.com/17781660/80863354-e0f70900-8ca5-11ea-9ace-77eb6df938b7.png)
![Output](https://user-images.githubusercontent.com/17781660/80863355-e2283600-8ca5-11ea-8477-9a3ec543f444.png)


## Soal 3
Source Code : [source](https://github.com/DSlite/SoalShiftSISOP20_modul4_T08/blob/master/ssfs.c)

### Deskripsi
Soal meminta kami untuk membuat Sinkronisasi direktori otomatis:
Tanpa mengurangi keumuman, misalkan suatu directory bernama dir akan tersinkronisasi dengan directory yang memiliki nama yang sama dengan awalan sync_ yaitu sync_dir. Dengan persyaratan :
* Kedua directory memiliki parent directory yang sama.
* Kedua directory kosong atau memiliki isi yang sama. Dua directory dapat dikatakan memiliki isi yang sama jika memenuhi:
  * Nama dari setiap berkas di dalamnya sama.
  * Modified time dari setiap berkas di dalamnya tidak berselisih lebih dari 0.1 detik.
* Sinkronisasi dilakukan ke seluruh isi dari kedua directory tersebut, tidak hanya di satu child directory saja.
* Sinkronisasi mencakup pembuatan berkas/directory, penghapusan berkas/directory, dan pengubahan berkas/directory.

***NOTE:*** Jika persyaratan di atas terlanggar, maka kedua directory tersebut tidak akan tersinkronisasi lagi.
Implementasi dilarang menggunakan symbolic links dan thread.

### Asumsi Soal
Disini untuk melakukan sinkronisasi terhadap directory, kami melakukannya dengan cara untuk beberapa *system-call*, ketika dijalankan akan menjalankan system-call tersebut pada directory yang memiliki awalan `/sync_` pada parent directory yang sama. *system-call* yang melakukan proses tersebut adalah `_mkdir()`, `_unlink()`, `_rmdir()`, `_create()`, `_write()` yang dimana *system-call* tersebut untuk operasi *write* dan *delete* (ketika read hanya read 1 file).

### Pembahasan

Seperti yang dijelaskan pada asumsi soal, untuk membantu masing-masing *system-call* tersebut, maka kami membuat fungsi `nextSync()` yang digunakan untuk mengiterasikan directory dari `path` "sync" berikutnya yang diinputkan. Sebagai contoh, jika `path` yang diinputkan kedalam `nextSync()` adalah `/dir1/dir2`, maka akan menjadi `/sync_dir1/dir2`. Lalu jika `path` baru  tersebut dimasukkan kembali kedalam `nextSync()` akan menjadi `/dir1/sync_dir2`. Dan jika dimasukkan kembali akan menjadi `/sync_dir1/sync_dir2`. Dan jika dimasukkan kembali akan menjadi directory `path` awal.

#### Fungsi nextSync

```c
void nextSync(char *syncDirPath) {
  char buff[1000];
  char construct[1000];
  memset(construct, 0, sizeof(construct));
  strcpy(buff, syncDirPath);
  int state = 0;
  char *token = strtok(buff, "/");
  while (token != NULL) {
    char tBuff[1000];
    char get[1000];
    strcpy(tBuff, token);
    if (!state) {
      if (strstr(tBuff, "sync_")-tBuff == 0) {
        strcpy(get, tBuff+5);
      } else {
        sprintf(get, "sync_%s", tBuff);
        state = 1;
      }
    } else {
      strcpy(get, tBuff);
    }
    sprintf(construct, "%s/%s", construct, get);
    token = strtok(NULL, "/");
  }
  memset(syncDirPath, 0, 1000);
  sprintf(syncDirPath, "%s", construct);
}
```

Pertama, akan mendeklarasikan variable `buff[]` untuk menyimpan `syncDirPath` yang diinputkan. dan juga mendeklarasikan variable `construct[]` untuk menyimpan hasil konstruksi dari directory. Lalu terdapat variable `state` untuk mengecek apakah sudah terjadi perubahan salah satu directory menjadi `/sync_`. Lalu akan mendeklarasikan variable `*token` sebagai token untuk masing-masing nama directory `buff` menggunakan delimiter `"/"` dengan fungsi `strtok()`.\

Lalu untuk masing-masing nama directory yang tersimpan dalam `token` akan di loop. Untuk masing-masing loop, pertama akan mendeklarasikan variable `tBuff[]` untuk menyimpan string yang ditunjuk `token`. Lalu dicek apakah `state` masih belum terjadi perubahan khusus tersebut. Jika iya, maka akan dicek apakah `tBuff` berawalan `"sync_"` atau tidak, jika berawalan `"sync_"` maka akan di `strcpy()` ke variable `get` bagian nama directorynya saja. Jika nama directory `tBuff` tidak berawalan `sync_` maka akan di `sprintf()` kedalam variable `get` dengan menambahkan `"sync_"` pada awal string, lalu `state` diganti menjadi 1.\

Jika `state` sudah berubah, maka `tBuff` akan langsung di `strcpy()` ke dalam `get`. Setelah variable `get` didapat, maka akan dikonstruksi menggunakan `sprintf` ke dalam variable `construct`. dan `token` akan dilanjutkan.\

Setelah semua directory dari `syncDirPath` telah dikonstruksi ulang, maka syncDirPath akan `sprintf()` menjadi variable `construct`.\

Pada dasarnya, konsep yang kami gunakan yaitu `adisi bitwise secara terbalik`. Jika sebuah directory berawalan `"sync_"` maka diasumsikan directory tersebut bernilai bit '1', dan jika sebuah directory tidak berawalan `"sync_"` maka diasumsikan directory tersebut bernilai bit '0'. Jadi pada kasus `"/dir1/dir2"` -> `00` dan dilakukan adisi terbalik sehingga menjadi `10` -> `"/sync_dir1/dir2"`. Lalu jika path tersebut diinput kembali maka `"/sync_dir1/dir2"` -> `10` dilakukan adisi terbalik menjadi `01` -> `"/dir1/sync_dir2"`. Lalu jika dilakukan kembali `"/dir1/sync_dir2"` -> `01` dilakukan adisi terbalik menjadi `11` -> `"/sync_dir1/sync_dir2"`. Lalu jika dilakukan kembali `"/sync_dir1/sync_dir2"` -> `11` dilakukan adisi terbalik menjadi `00` -> `"/dir1/dir2"`.

#### Contoh System-call Membutuhkan Sinkronisasi

```c
static int _rmdir(const char *path) {
	char fpath[1000];
	changePath(fpath, path, 0, 0);
	int res;

	res = rmdir(fpath);

  char syncOrigPath[1000];
  char syncDirPath[1000];
  char syncFilePath[1000];
  memset(syncOrigPath, 0, sizeof(syncOrigPath));
  strcpy(syncOrigPath, path);
  getDirAndFile(syncDirPath, syncFilePath, syncOrigPath);
  memset(syncOrigPath, 0, sizeof(syncOrigPath));
  strcpy(syncOrigPath, syncDirPath);
  do {
    char syncPath[1000];
    memset(syncPath, 0, sizeof(syncPath));
    nextSync(syncDirPath);
    if (strcmp(syncDirPath, syncOrigPath) == 0) break;
    changePath(syncPath, syncDirPath, 0, 0);
    if (access(syncPath, F_OK) == -1) continue;
    sprintf(syncPath, "%s/%s", syncPath, syncFilePath);
    rmdir(syncPath);
  } while (1);

  const char *desc[] = {path};
  logFile("WARNING", "RMDIR", res, 1, desc);

	if (res == -1) return -errno;

	return 0;
}
```

Sebagai contoh, pada *system-call* `_rmdir()`. Setelah *system-call* utama dijalankan pada `fpath`, kami membuat `syncOrigPath` untuk menyimpan `path` directory awal, `syncDirPath` untuk menyimpan `path` directory masing-masing iterasi, dan `syncFilePath` untuk menyimpan nama file. Pertama `path` akan di `strcpy()` kedalam `syncOrigPath`. Setelah itu, akan dilakukan fungsi `getDirAndFile()` untuk memisahkan directory dan nama filenya kedalam variable `syncDirPath` dan `syncFilePath`. Setelah itu `syncOrigPath` akan diset ulang menjadi `syncDirPath` (path awal).\

Lalu akan dilakukan `do-while(1)` loop untuk melakukan iterasi. Pertama akan deklarasi variable `syncPath` untuk menyimpan absolute `path` dari `syncDirPath` pada iterasi tersebut. Lalu `syncDirPath` akan dilakukan fungsi `nextSync()` untuk mendapatkan `syncDirPath` iterasi berikutnya. Lalu dicek apakah `syncDirPath` sama dengan `syncOrigPath` menggunakan `strcmp`, dan jika sama `while` loop akan di `break`. Lalu akan melakukan fungsi `changePath` untuk memasukkan **`mount-point`** pada `syncDirPath`. Karena kita tidak tahu apakah path `syncDirPath` tersebut ada, maka akan kita coba `access` terlebih dahulu, jika ternyata tidak ada, maka akan di `continue`. Setelah itu `syncPath` dan `syncFilePath` akan dijadikan satu dan *system-call* awal akan dipanggil terhadap `syncPath`.\

Untuk *system-call* yang telah dijelaskan pada asumsi-soal juga akan menggunakan proses yang sama seperti ini.

### Kesulitan
Tidak ada.

### ScreenShot
![Output](https://user-images.githubusercontent.com/17781660/80864806-3e438800-8caf-11ea-942b-0065740cec63.png)

## Soal 4
Source Code : [source](https://github.com/DSlite/SoalShiftSISOP20_modul4_T08/blob/master/ssfs.c)

### Deskripsi
Soal meminta kami untuk membuat sebuah log system yang akan membuat sebuah log file bernama "fs.log" di direktori *home* user yang berguna untuk menyimpan daftar perintah system call yang telah dijalankan. Disini pencatatan pada log dibagi menjadi dua level yaitu **WARNING** untuk system call `rmdir`& `unlink`dan **INFO** untuk system call sisanya.\

Dengan format logging: `[LEVEL]::[yy][mm][dd]-[HH]:[MM]:[SS]::[CMD]::[DESC ...]`
* *yy*,*mm*,*dd* akan merepresentasikan tanggal systemcall terpanggil
* *HH*,*MM*, *SS* akan merepresantasikan waktu systemcall terpanggil
* *CMD*, *DESC* akan merepresantasikan system call yang terpanggil dan path file yang di eksekusi dengan system call tersebut

### Asumsi Soal
Kami mengasumsikan bahwa akan ada sebuah fungsi yang akan dipanggil setiap akan melakukan log pada setiap *system-call* dengan format yang telah ditentukan. Setiap *system-call* akan menginputkan ***LEVEL***, ***CMD***, ***res*** dari *system-call* nya (tambahan), dan ***DESC*** (`path`) yang diinputkan pada *system-call* tersebut. Sebagai tambahan, untuk log pada soal 1 dan soal 2 akan menggunakan ***LEVEL*** `SPECIAL` dengan ***CMD*** berupa metode enkripsinya.

### Pembahasan

Fungsi yang akan digunakan untuk melakukan logging adalah fungsi `logFile()`.

```c
static const char *logpath = "/home/umum/Documents/SisOpLab/fuse_log.txt";
```

Pertama kami melakukan pendefisinian path dari file log yang akan diproses oleh fungsi `logFile()`.

#### Fungsi logFile

``` c
void logFile(char *level, char *cmd, int res, int lenDesc, const char *desc[]) {
  FILE *f = fopen(logpath, "a");
  time_t t;
  struct tm *tmp;
  char timeBuff[100];

  time(&t);
  tmp = localtime(&t);
  strftime(timeBuff, sizeof(timeBuff), "%y%m%d-%H:%M:%S", tmp);

  fprintf(f, "%s::%s::%s::%d", level, timeBuff, cmd, res);
  for (int i = 0; i < lenDesc; i++) {
    fprintf(f, "::%s", desc[i]);
  }
  fprintf(f, "\n");

  fclose(f);
}
```

Fungsi ini akan menuliskan pada log file sesuai dengan format yang sudah di tentukan. Pertama, fungsi ini memerlukan 5 buah arguments ketika dipanggil.
* **`level`** untuk mendefinisikan level dari atribut yang berjalan *(INFO/WARNING/SPECIAL)*.
* **`cmd`** untuk mendefinisikan command *system-call* yang terpanggil.
* **`res`** untuk mendefinisikan result dari *system-call* yang terpanggil.
* **`lenDesc`** untuk menyimpan banyaknya ***DESC*** yang akan ditulis dalam log.
* **`*desc[]`** untuk menyimpan array of string dari ***DESC*** yang akan ditulis.

Pertama, fungsi akan membuka dan membuat (jika belum ada) file log pada `logpath` menggunakan `fopen()` kedalam pointer FILE `*f`. Lalu akan dicari waktu saat itu menggunakan variable `t` dan `*tmp` dan akan disimpan dalam bentuk string pada `timeBuff`. Lalu dijalankan fungsi `time()` pada alamat `t` untuk menyimpan timestamp saat ini. Lalu `t` akan diubah menjadi `struct tm` menggunakan fungsi `localtime()` kedalam variable `tmp`. Lalu menggunakan fungsi `strftime()` untuk memformat `tmp` menjadi waktu yang telah terformat ke dalam variable `timeBuff`. Setelah itu, `level`, `timeBuff`, `cmd`, dan `res` akan diprint kedalam file `f` menggunakan fungsi `fprintf()`. Lalu untuk masing-masing `desc` akan diiterasi menggunakan `lenDesc`, dimana masing-masing iterasi akan memasukkan `desc[i]` kedalam `f` dengan format yang sesuai. Lalu terakhir akan memasukkan `"\n"` diakhir log. Setelah log selesai, `f` akan di `fclose()`. Lalu untuk seluruh *system-call* akan memanggil fungsi `logFile()` tersebut untuk melakukan logging.

### Kesulitan
Tidak ada.

### ScreenShot
![Output](https://user-images.githubusercontent.com/17781660/80865229-2e797300-8cb2-11ea-82c7-682994c5cf27.png)
![Output](https://user-images.githubusercontent.com/17781660/80865231-2faaa000-8cb2-11ea-996c-72b9166bc99f.png)
