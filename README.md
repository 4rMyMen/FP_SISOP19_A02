# LAPORAN PENJELASAN FP SISOP 2019
## Soal Daemon+Thread

Buatlah program C yang menyerupai crontab menggunakan daemon dan thread. Ada sebuah file crontab.data untuk menyimpan config dari crontab. Setiap ada perubahan file tersebut maka secara otomatis program menjalankan config yang sesuai dengan perubahan tersebut tanpa perlu diberhentikan. Config hanya sebatas * dan 0-9 (tidak perlu /,- dan yang lainnya)

### Jawab :
#### Penjelasan Structure yg dipakai

  * Dibuatkan strktur untuk menyimpan satuan command yang dijalankan berserta waktu eksekusinya.
  * Dibuatkan struktur untuk menyimpan isi dari file config. Menyimpan command2 yg dijalankan dan waktu pembukaan/modifikasi file.
  
#### Membuat Fungsi Pembaca File

  * File dibaca per kata pada setiap line. Setiap line diparsing dan dicek jika valid. Jika valid maka akan dimasukkan ke array 
  command yang dijalankan pada struct config
 
#### Membuat Fungsi Parsing Instruksi 

  * Setiap line di-parsing kata per kata dengan urutan dari waktu eksekusi sampai ke program yg dijalankan.
  
#### Membuat Fungsi Pengecek Waktu Eksekusi
  
  * Thread digunakan untuk mengecek waktu eksekusi program dan sekaligus mengeksekusi program jika sudah waktunya
 
#### Membuat Fungsi Pengecek Modifikasi File
  
  * Pemodifikasian file dicek dengan melihat waktu modifikasi file. Jika ada perubahan waktu modifikasi file, fungsi pembaca akan dijalankan lagi. Fungsi ini dijalankan di while loop daemon.
 
 
 
 
 
