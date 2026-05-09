#pragma once
#include <glm/glm/glm.hpp>
#include <string>
#include <vector>

// Data edukasi tiap objek
struct ObjectInfo {
    std::string id;
    std::string name;
    std::string subtitle;
    glm::vec3   worldPos;       // posisi objek di world
    float       triggerRadius;  // jarak mulai muncul prompt
    
    // Konten popup (maks 4 slide)
    struct Slide {
        std::string icon;    // emoji / simbol
        std::string title;
        std::string body;    // max ~200 karakter
        std::string fact;    // "Fun fact" satu baris
    };
    std::vector<Slide> slides;
};

class InteractionSystem {
public:
    // State
    bool  promptVisible   = false;   // tampilkan "Tekan E"
    bool  popupOpen       = false;   // popup edukatif terbuka
    int   currentSlide    = 0;
    int   nearestIdx      = -1;

    // Animasi
    float promptAlpha     = 0.0f;
    float popupAlpha      = 0.0f;
    float promptScale     = 1.0f;
    bool  popupClosing    = false;

    std::vector<ObjectInfo> objects;

    void init() {
        // ── Mitokondria ──────────────────────────────────
        ObjectInfo mito;
        mito.id            = "mitokondria";
        mito.name          = "Mitokondria";
        mito.subtitle      = "Pusat Energi Sel";
        mito.worldPos      = glm::vec3(-6.0f, 1.5f, -5.0f);
        mito.triggerRadius = 5.5f;
mito.slides = {        
    {
        "⚡", "Penghasil Energi",
        "Mitokondria adalah organel penghasil energi sel. "
        "Organel ini mengubah glukosa dan oksigen menjadi ATP, "
        "molekul energi yang digunakan sel untuk beraktivitas. "
        "Sel otot dan saraf memiliki ribuan mitokondria karena "
        "kebutuhan energinya yang sangat tinggi.",
        "Satu sel jantung memiliki lebih dari 5.000 mitokondria!"
    },
    {
        "🧬", "Struktur Unik",
        "Mitokondria memiliki dua lapis membran. Membran dalamnya "
        "berlipat-lipat membentuk krista untuk memperluas area "
        "produksi energi. Di dalamnya terdapat matriks tempat "
        "terjadinya siklus Krebs penghasil energi.",
        "Mitokondria punya DNA sendiri yang diwarisi dari ibu!"
    },
    {
        "🔬", "Proses Respirasi",
        "Respirasi seluler terjadi dalam 4 tahap: glikolisis, "
        "dekarboksilasi, siklus Krebs, dan rantai transpor elektron. "
        "Satu molekul glukosa menghasilkan 36-38 molekul ATP. "
        "Proses ini membutuhkan oksigen dan menghasilkan CO2.",
        "ATP diproduksi di membran dalam mitokondria!"
    },
    {
        "💡", "Asal Usul Unik",
        "Teori endosimbiotik menyatakan mitokondria dulunya adalah "
        "bakteri yang hidup bebas. Sekitar 1,5 miliar tahun lalu, "
        "bakteri ini 'ditelan' oleh sel inang dan bersimbiosis. "
        "Keduanya saling menguntungkan hingga menjadi satu organel.",
        "Mitokondria bereproduksi seperti bakteri: dengan membelah diri!"
    },
    {
        "🩺", "Gangguan Mitokondria",
        "Kerusakan mitokondria dapat menyebabkan kelelahan kronis, "
        "kelemahan otot, gangguan saraf, dan penyakit degeneratif. "
        "Karena diwariskan dari ibu, penyakit ini menurun secara "
        "maternal. Terapi penggantian mitokondria kini mulai dikembangkan.",
        "Penyakit mitokondria sulit didiagnosis karena gejalanya bervariasi!"
    }
};       objects.push_back(mito);

        // ── Nukleus ──────────────────────────────────────
// ── Nukleus ──────────────────────────────────────────
ObjectInfo nuk;
nuk.id            = "nukleus";
nuk.name          = "Nukleus";
nuk.subtitle      = "Pusat Komando Sel";
nuk.worldPos      = glm::vec3(6.0f, 1.5f, -5.0f);
nuk.triggerRadius = 5.5f;
nuk.slides = {
    {
        "1",
        "Pusat Komando Sel",
        "Nukleus adalah organel terbesar yang menjadi pusat kontrol sel. "
        "Di dalamnya tersimpan DNA, materi genetik yang mengatur "
        "seluruh aktivitas sel dan pewarisan sifat. Tanpa nukleus, "
        "sel tidak bisa berkembang biak dan akan mati.",
        "DNA manusia jika direntangkan panjangnya mencapai 2 meter!"
    },
    {
        "2",
        "Membran & Pori Nukleus",
        "Nukleus dilindungi membran ganda yang disebut selubung nukleus. "
        "Membran ini memiliki pori-pori yang mengatur keluar masuknya "
        "molekul seperti RNA dan protein. Pori ini sangat selektif "
        "hanya melewatkan molekul tertentu yang memiliki 'tiket'.",
        "Setiap nukleus memiliki sekitar 3.000 hingga 4.000 pori!"
    },
    {
        "3",
        "Proses Transkripsi DNA",
        "Saat sel perlu membuat protein, DNA di nukleus dibaca menjadi "
        "mRNA dalam proses yang disebut transkripsi. mRNA yang terbentuk "
        "kemudian keluar melalui pori nukleus menuju ribosom, tempat "
        "protein dirakit sesuai kode genetik.",
        "Sel manusia mentranskripsi ribuan gen setiap detiknya!"
    },
    {
        "4",
        "Nukleolus Pabrik Ribosom",
        "Di dalam nukleus terdapat nukleolus, struktur padat tanpa membran. "
        "Nukleolus bertugas memproduksi rRNA dan merakit subunit ribosom. "
        "Ribosom yang sudah jadi akan diekspor ke sitoplasma untuk "
        "mensintesis protein sesuai perintah DNA.",
        "Sel yang aktif membuat protein memiliki nukleolus yang lebih besar!"
    },
    {
        "5",
        "Pembelahan Inti Sel",
        "Saat sel membelah, nukleus juga ikut membelah melalui proses "
        "mitosis. Kromosom yang mengandung DNA akan digandakan lalu "
        "dibagi rata ke dua sel anak. Kesalahan pembelahan dapat "
        "menyebabkan kelainan genetik seperti Down syndrome.",
        "Sel hati manusia bisa memiliki dua nukleus dalam satu sel!"
    }
};objects.push_back(nuk);
    }

    // Panggil tiap frame — update state proximity
    void update(glm::vec3 cameraPos, float deltaTime) {
        nearestIdx     = -1;
        float minDist  = 9999.0f;

        for (int i = 0; i < (int)objects.size(); i++) {
            float d = glm::distance(cameraPos, objects[i].worldPos);
            if (d < objects[i].triggerRadius && d < minDist) {
                minDist    = d;
                nearestIdx = i;
            }
        }

        // Fade prompt in/out
        bool shouldShow = (nearestIdx >= 0 && !popupOpen);
        if (shouldShow) {
            promptAlpha = std::min(1.0f, promptAlpha + deltaTime * 3.0f);
            // Pulse scale
            promptScale = 1.0f + sinf((float)glfwGetTime() * 4.0f) * 0.04f;
        } else {
            promptAlpha = std::max(0.0f, promptAlpha - deltaTime * 4.0f);
        }
        promptVisible = promptAlpha > 0.01f;

        // Fade popup
        if (popupOpen && !popupClosing) {
            popupAlpha = std::min(1.0f, popupAlpha + deltaTime * 5.0f);
        }
        if (popupClosing) {
            popupAlpha = std::max(0.0f, popupAlpha - deltaTime * 6.0f);
            if (popupAlpha <= 0.0f) {
                popupOpen    = false;
                popupClosing = false;
            }
        }
    }

    // Tekan E
    void onInteract() {
        if (popupOpen) return;
        if (nearestIdx < 0) return;
        popupOpen    = true;
        popupClosing = false;
        currentSlide = 0;
        popupAlpha   = 0.0f;
    }

    // Tekan -> (next slide) atau <- (prev slide)
    void nextSlide() {
        if (!popupOpen) return;
        int total = (int)objects[nearestIdx].slides.size();
        if (currentSlide < total - 1) currentSlide++;
    }
    void prevSlide() {
        if (!popupOpen) return;
        if (currentSlide > 0) currentSlide--;
    }

    // Tutup popup
    void closePopup() {
        if (!popupOpen) return;
        popupClosing = true;
    }

    ObjectInfo* getNearestObject() {
        if (nearestIdx < 0) return nullptr;
        return &objects[nearestIdx];
    }
};