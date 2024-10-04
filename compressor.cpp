#include <fstream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const int width = 60;  // 60 px p/row, cada px lleva 2 bit de info, 120 bits p/row, 128 redondeando
const int height = 45; // 45 cols, -> 128 bits -> 16bytes -> 16*45 = 720 bytes p/ frame
const int frame_c = 5258; // 5258 frames -> 720 bytes p/frame -> 3785760 bytes (3.78576 MB o 3.61038 MiB)

void printBits(uint8_t x) {
  for (int i = 7; i >= 0; --i)
    std::printf("%d", ((x & (1 << i)) >> i));
}

union FourPixels {
  struct {
    uint8_t p4 : 2;
    uint8_t p3 : 2;
    uint8_t p2 : 2;
    uint8_t p1 : 2;
  };
  uint8_t raw;
};

typedef std::vector<std::vector<uint8_t>> FrameData;
void writeFrame(std::fstream& file, const FrameData& data) {
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; x+= 4) {
      // for (int i = 1; i >= 0; --i)
        // std::printf("%d", ((data[y][x] & (1 << i)) >> i));
      FourPixels to_write {0};

      to_write.p1 = data[y][x];
      to_write.p2 = data[y][x+1];
      to_write.p3 = data[y][x+2];
      to_write.p4 = data[y][x+3];

      // printBits(to_write.raw);
      // std::printf("\n");
      file.write((char*)&to_write, sizeof(to_write));
    }
  }
}

int main(int argc, char* argv[]) {


  int c = 0;
  std::fstream out {"./out0.bapple", std::ios::out | std::ios::binary};
  for (int i = 1; i <= frame_c; ++i) {
    // if (i%1500 == 0) {
    //   ++c;
    //   out.close();
    //   out = std::fstream("./out"+std::to_string(c)+".bapple", std::ios::out | std::ios::binary);
    // }
    char str[9];
    snprintf(str, 9, "%04d.png", i);
    std::string path {"./frames/"};
    path += str;
    // std::string path {"./raymoo.png"};

    int w, h, ch;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &h, 1);

    FrameData frame;
    unsigned char* curr = data;
    for (int y = 0; y < height; ++y) {
      frame.push_back(std::vector<uint8_t>());
      for (int x = 0; x < width; ++x)
        frame[y].push_back(*curr++ >> 6);
    }

    stbi_image_free(data);

    writeFrame(out, frame);
  }
  out.close();
}
