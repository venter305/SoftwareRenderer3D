#include <iostream>
#include <fstream>
#include <iomanip>

struct PNGImage {
  uint8_t *data = nullptr;
  int width,height;
  bool hasTransparency = false;

  PNGImage() = default;

  PNGImage(const PNGImage &p){
    width = p.width;
    height = p.height;
    hasTransparency = p.hasTransparency;
    delete[] data;
    data = new uint8_t[width*height*4];
    memcpy(data,p.data,width*height*4);
  }
  ~PNGImage(){
    delete[] data;
  }
};

int ReadPNGFile(std::string path, PNGImage &image){
  std::ifstream file;
  file.open(path);
  if (!file)
    return 0;

  char header[8];
  file.read(header,8);

  std::vector<uint8_t> data;
  int bitDepth,colorType,compressionType,filterType,interlaceType;

  while(!file.eof()){
    uint32_t length;
    char lenBuf[4];
    file.read(lenBuf,4);
    length = (uint8_t)lenBuf[0]*0x1000000+(uint8_t)lenBuf[1]*0x10000+(uint8_t)lenBuf[2]*0x100+(uint8_t)lenBuf[3];
    char chunk[5];
    file.read(chunk,4);
    chunk[4] = '\0';

    if (!strcmp(chunk,"IHDR")){
      char buf[4];
      file.read(buf,4);
      image.width = (uint8_t)buf[0]*0x1000000+(uint8_t)buf[1]*0x10000+(uint8_t)buf[2]*0x100+(uint8_t)buf[3];
      file.read(buf,4);
      image.height = (uint8_t)buf[0]*0x1000000+(uint8_t)buf[1]*0x10000+(uint8_t)buf[2]*0x100+(uint8_t)buf[3];
      file.read(buf,1);
      bitDepth = (uint8_t)buf[0];
      file.read(buf,1);
      colorType = (uint8_t)buf[0];
      file.read(buf,1);
      compressionType = (uint8_t)buf[0];
      file.read(buf,1);
      filterType = (uint8_t)buf[0];
      file.read(buf,1);
      interlaceType = (uint8_t)buf[0];

      // std::cout << "Width: " <<  image.width << std::endl;
      // std::cout << "Heght: " <<  image.height << std::endl;
      // std::cout << "Bit Depth: " <<  bitDepth << std::endl;
      // std::cout << "Color Type: " <<  colorType << std::endl;
      // std::cout << "Compression Type: " <<  compressionType << std::endl;
      // std::cout << "Filter Type: " <<  filterType << std::endl;
      // std::cout << "InterlaceType: " <<  interlaceType << std::endl;
      // std::cout << std::endl;
    }
    else if (!strcmp(chunk,"IDAT")){
      for (int i=0;i<length;i++){
        char buf;
        file.read(&buf,1);
        data.push_back(buf);
      }
      // std::cout << length << std::endl;
      //file.ignore(length);
    }
    else if (!strcmp(chunk,"IEND"))
      break;
    else
      file.ignore(length);

    //crc
    file.ignore(4);
  }

  int numBytes = 4*image.width*image.height;
  image.data = new uint8_t[numBytes];

  int numChannelBytes = (bitDepth==16?2:1);
  int numChannels = 1;
  switch(colorType){
    case 0: //grayscale
      numChannels = 1;
      break;
    case 2: //rgb
      numChannels = 3;
      break;
    case 3: //indexed (pallet)
      numChannels = 1;
      break;
    case 4: //grayscale and alpha
      numChannels = 2;
      break;
    case 6: //rgba
      numChannels = 4;
      break;
  }
  std::vector<uint8_t> compressedData;
  int index = 2;
  std::vector<uint8_t> deCompressedData;
  bool final = false;
  //file.ignore(2); //zlib header
  for (int i=0;i<data.size();i++){
    compressedData.push_back(data[index++]);
  }


  auto shiftVectorLeft = [&](std::vector<uint8_t> &vector,int numShift){
    for (int i=0;i<numShift;i++){
      vector[0] = vector[0]<<1;
      for (int j=1;j<vector.size();j++){
        vector[j-1] += (vector[j]&0x80)>>7;
        vector[j] = vector[j]<<1;
      }
    }
  };

  int currentByte = 0;
  int currentBitOffset = 0;
  auto GetBits = [&](std::vector<uint8_t> &vector,int numBits,bool swapBits=false){
    int value = 0;
    for (int i=0;i<numBits;i++){
      bool bit = 0;
      if (currentByte >= vector.size())
        return -1;
      bit = vector[currentByte]&(1<<currentBitOffset++);
      if (currentBitOffset >= 8){
        currentBitOffset = 0;
        currentByte++;
      }
      if(!swapBits)value |= bit << i;
      else value = (value<<1) | bit;
    }
    return value;
  };

  auto SwapBitOrder = [](uint8_t byte){
    uint8_t out = 0;
    for (int i=0;i<7;i++){
      out += (bool)(byte&(1<<i));
      out <<= 1;
    }
    return out;
  };
  index = 2;

  //This is a complex test, test test test test
  //data = {120,156,93,196,225,9,0,16,24,4,208,85,110,0,227,88,64,186,162,136,186,239,135,241,79,126,170,215,203,173,11,87,65,93,115,15,30,4,21,233,253,101,82,70,15,186};
  //Test
  //data = {120,156,5,192,49,17,0,0,0,1,192,146,42,72,64,253,225,47,221,1,3,221,1,161};
  //data = {120,156,109,202,33,1,0,0,8,196,192,172,32,136,64,254,119,135,225,236,86,52,203,213,225,251,2,223,159,28,113};
  GetBits(data,16);
  while(!final){
    final = GetBits(data,1);//SwapBitOrder(data[index])&0x80;
    int type = GetBits(data,2);
    //std::cout << final << ' ' << type << std::endl;
    if (type == 0){
      GetBits(data,5);
      int len = GetBits(data,16);//data[++index]+data[++index]*0x100;
      int nlen = GetBits(data,16);//data[++index]+data[++index]*0x100;
      for (int i=0;i<len;i++){
        deCompressedData.push_back(GetBits(data,8));
      }
      //GetBits(data,len*8);
      //index += 1+len;
    }
    else if (type == 1){ //fixed huffman coding
      int value = -1;
      while(value != 0){
        value = 0;
        for (int i=0;i<7;i++){
          value <<= 1;
          value += GetBits(data,1);
        }
        if (value >= 0 && value <= 0x17){ //256-279
          int tmp = 256+value;
          int len = 0;
          if (tmp == 256)
            break;
          else if (tmp >= 257 && tmp <= 264){
            len = 3+tmp-257;
          }
          else if (tmp >= 265 && tmp <= 268){
            len = 11+GetBits(data,1)+(tmp-265)*std::pow(2,1);
          }
          else if (tmp >= 269 && tmp <= 272){
            len = 19+GetBits(data,2)+(tmp-269)*std::pow(2,2);
          }
          else if (tmp >= 273 && tmp <= 276){
            len = 35+GetBits(data,3)+(tmp-273)*std::pow(2,3);
          }
          else if (tmp >= 277 && tmp <= 280){
            len = 67+GetBits(data,4)+(tmp-277)*std::pow(2,4);
          }

          int dist = 0;
          int distCode = GetBits(data,5,true);
          if (distCode >= 0 && distCode <= 3){
            dist = 1+distCode;
          }
          else if (distCode >= 4 && distCode <= 5){
            dist = 5+GetBits(data,1)+(distCode-4)*std::pow(2,1);
          }
          else if (distCode >= 6 && distCode <= 7){
            dist = 9+GetBits(data,2)+(distCode-6)*std::pow(2,2);
          }
          else if (distCode >= 8 && distCode <= 9){
            dist = 17+GetBits(data,3)+(distCode-8)*std::pow(2,3);
          }
          else if (distCode >= 10 && distCode <= 11){
            dist = 33+GetBits(data,4)+(distCode-10)*std::pow(2,4);
          }
          else if (distCode >= 12 && distCode <= 13){
            dist = 65+GetBits(data,5)+(distCode-12)*std::pow(2,5);
          }
          else if (distCode >= 14 && distCode <= 15){
            dist = 129+GetBits(data,6)+(distCode-14)*std::pow(2,6);
          }
          else if (distCode >= 16 && distCode <= 17){
            dist = 257+GetBits(data,7)+(distCode-16)*std::pow(2,7);
          }
          else if (distCode >= 18 && distCode <= 19){
            dist = 513+GetBits(data,8)+(distCode-18)*std::pow(2,8);
          }
          else if (distCode >= 20 && distCode <= 21){
            dist = 1025+GetBits(data,9)+(distCode-20)*std::pow(2,9);
          }
          else if (distCode >= 22 && distCode <= 23){
            dist = 2049+GetBits(data,10)+(distCode-22)*std::pow(2,10);
          }
          else if (distCode >= 24 && distCode <= 25){
            dist = 4097+GetBits(data,11)+(distCode-24)*std::pow(2,11);
          }
          else if (distCode >= 26 && distCode <= 27){
            dist = 8193+GetBits(data,12)+(distCode-26)*std::pow(2,12);
          }
          else if (distCode >= 28 && distCode <= 29){
            dist = 16385+GetBits(data,13)+(distCode-28)*std::pow(2,13);
          }
          int offset = deCompressedData.size()-dist;
          for (int i=0;i<len;i++){
            deCompressedData.push_back(deCompressedData[(offset+i)]);
          }
          continue;
        }
        value = (value<<1) | GetBits(data,1);
        if (value >= 0x30 && value <= 0xBF){ //0-143
          deCompressedData.push_back(value-0x30);
          continue;
        }
        if (value >= 0xC0 && value <= 0xC7){ //280-287
          int tmp = 280+value-0xC0;
          int len = 0;
          if (tmp >= 277 && tmp <= 280){
            len = 67+GetBits(data,4)+(tmp-277)*std::pow(2,4);
          }
          else if (tmp >= 281 && tmp <= 284){
            len = 131+GetBits(data,5)+(tmp-281)*std::pow(2,5);
          }
          else if (tmp == 285){
            len = 258;
          }
          int dist = 0;
          int distCode = GetBits(data,5,true);
          if (distCode >= 0 && distCode <= 3){
            dist = 1+distCode;
          }
          else if (distCode >= 4 && distCode <= 5){
            dist = 5+GetBits(data,1)+(distCode-4)*std::pow(2,1);
          }
          else if (distCode >= 6 && distCode <= 7){
            dist = 9+GetBits(data,2)+(distCode-6)*std::pow(2,2);
          }
          else if (distCode >= 8 && distCode <= 9){
            dist = 17+GetBits(data,3)+(distCode-8)*std::pow(2,3);
          }
          else if (distCode >= 10 && distCode <= 11){
            dist = 33+GetBits(data,4)+(distCode-10)*std::pow(2,4);
          }
          else if (distCode >= 12 && distCode <= 13){
            dist = 65+GetBits(data,5)+(distCode-12)*std::pow(2,5);
          }
          else if (distCode >= 14 && distCode <= 15){
            dist = 129+GetBits(data,6)+(distCode-14)*std::pow(2,6);
          }
          else if (distCode >= 16 && distCode <= 17){
            dist = 257+GetBits(data,7)+(distCode-16)*std::pow(2,7);
          }
          else if (distCode >= 18 && distCode <= 19){
            dist = 513+GetBits(data,8)+(distCode-18)*std::pow(2,8);
          }
          else if (distCode >= 20 && distCode <= 21){
            dist = 1025+GetBits(data,9)+(distCode-20)*std::pow(2,9);
          }
          else if (distCode >= 22 && distCode <= 23){
            dist = 2049+GetBits(data,10)+(distCode-22)*std::pow(2,10);
          }
          else if (distCode >= 24 && distCode <= 25){
            dist = 4097+GetBits(data,11)+(distCode-24)*std::pow(2,11);
          }
          else if (distCode >= 26 && distCode <= 27){
            dist = 8193+GetBits(data,12)+(distCode-26)*std::pow(2,12);
          }
          else if (distCode >= 28 && distCode <= 29){
            dist = 16385+GetBits(data,13)+(distCode-28)*std::pow(2,13);
          }
          //std::cout << len << ' ' << dist << ' ' << distCode << std::endl;
          int offset = deCompressedData.size()-dist;
          for (int i=0;i<len;i++){
            deCompressedData.push_back(deCompressedData[(offset+i)]);
          }
          continue;
        }
        value = (value<<1) | GetBits(data,1);
        if (value >= 0x190 && value <= 0x1FF){ //144-255
          deCompressedData.push_back(144+value-0x190);
          continue;
        }
      }
    }
    else if (type == 2){
      int numLitLenCodes = GetBits(data,5)+257;
      int numDistCodes = GetBits(data,5)+1;
      int numCodeLen = GetBits(data,4)+4;
      uint8_t codeLengths[19] = {0};
      codeLengths[16] = GetBits(data,3);
      codeLengths[17] = GetBits(data,3);
      codeLengths[18] = GetBits(data,3);
      codeLengths[0] = GetBits(data,3);
      for (int i=0;i<numCodeLen-4;i++){
        int j = (i % 2 == 0)?(8+i/2):(7-i/2);
        codeLengths[j] = GetBits(data,3);
      }

      auto MakeHuffmanCodes = [](int numLens,uint8_t *lengths,int *codes){
        int max = 0;
        for (int i=0;i<numLens;i++){
          if (lengths[i] > max)
            max = lengths[i];
        }

        int bl_count[max+1] = {0};
        for (int i=0;i<numLens;i++){
          bl_count[lengths[i]]++;
        }
        int next_code[max+1];
        int code = 0;
        bl_count[0] = 0;
        for (int i=1;i<=max;i++){
          code = (code+bl_count[i-1]) << 1;
          next_code[i] = code;
        }

        for (int i=0;i<numLens;i++){
          int len = lengths[i];
          int startbit = 1 << len;
          if (len != 0){
            codes[i] = startbit | next_code[len];
            next_code[len]++;
          }
          else
            codes[i] = -1;
        }
      };

      int codes[19];
      MakeHuffmanCodes(19,codeLengths,codes);
      // for (int i=0;i<19;i++)
      //   if(codes[i] != -1)std::cout << i << ' ' << codes[i] << std::endl;
      std::vector<int> codelens;

      //std::cout << (numLenCodes+numDistCodes) << std::endl;

      while(codelens.size() < numLitLenCodes+numDistCodes){
        uint32_t value = 1;
        int symbol = -1;
        bool match = false;
        while(!match){
          int bits = GetBits(data,1);
          value = (value<<1) + bits;
          //std::cout << value << std::endl;
          for (int j=0;j<19;j++){
            if (value == codes[j]){
              match = true;
              symbol = j;
            }
          }
        }
        if (symbol >= 0 && symbol <= 15){
          codelens.push_back(symbol);
        }
        else if (symbol == 16){
          int prevVal = codelens.back();
          int num = GetBits(data,2)+3;
          for (int j=0;j<num;j++){
            codelens.push_back(prevVal);
          }
        }
        else if (symbol == 17){
          int num = GetBits(data,3)+3;
          for (int j=0;j<num;j++){
            codelens.push_back(0);
          }
        }
        else if (symbol == 18){
          int num = GetBits(data,7)+11;
          for (int j=0;j<num;j++){
            codelens.push_back(0);
          }
        }
      }
      // for (int i=0;i<codelens.size();i++)
      //   if(codelens[i] != 0)std::cout << i << ' ' << codelens[i] << std::endl;

      int litLenCodes[numLitLenCodes];
      uint8_t litLenCodeLens[numLitLenCodes];
      for (int j=0;j<numLitLenCodes;j++)
        litLenCodeLens[j] = codelens[j];
      MakeHuffmanCodes(numLitLenCodes,litLenCodeLens,litLenCodes);

      int distCodes[numDistCodes];
      uint8_t distCodeLens[numDistCodes];
      for (int j=numLitLenCodes;j<numLitLenCodes+numDistCodes;j++)
        distCodeLens[j-numLitLenCodes] = codelens[j];
      MakeHuffmanCodes(numDistCodes,distCodeLens,distCodes);

      while(true){
        int value = 1;
        int symbol = -1;
        bool match = false;
        while(!match){
          int bits = GetBits(data,1);
          value = (value<<1) + bits;
          for (int j=0;j<numLitLenCodes;j++){
            if (value == litLenCodes[j]){
              match = true;
              symbol = j;
            }
          }
        }
        //std::cout << symbol << std::endl;
        if (symbol == 256)
          break;
        else if (symbol >= 0 && symbol <= 255)
          deCompressedData.push_back(symbol);
        else if (symbol >= 257 && symbol <= 285){
          int len = 0;
          if (symbol >= 257 && symbol <= 264){
            len = 3+symbol-257;
          }
          else if (symbol >= 265 && symbol <= 268){
            len = 11+GetBits(data,1)+(symbol-265)*std::pow(2,1);
          }
          else if (symbol >= 269 && symbol <= 272){
            len = 19+GetBits(data,2)+(symbol-269)*std::pow(2,2);
          }
          else if (symbol >= 273 && symbol <= 276){
            len = 35+GetBits(data,3)+(symbol-273)*std::pow(2,3);
          }
          else if (symbol >= 277 && symbol <= 280){
            len = 67+GetBits(data,4)+(symbol-277)*std::pow(2,4);
          }
          else if (symbol >= 281 && symbol <= 284){
            len = 131+GetBits(data,5)+(symbol-281)*std::pow(2,5);
          }
          else if (symbol == 285){
            len = 258;
          }

          int dist = 0;
          int distCode = -1;
          value = 1;
          match = false;
          while(!match){
            int bits = GetBits(data,1);
            value = (value<<1) + bits;
            for (int j=0;j<numDistCodes;j++){
              if (value == distCodes[j]){
                match = true;
                distCode = j;
              }
            }
          }

          if (distCode >= 0 && distCode <= 3){
            dist = 1+distCode;
          }
          else if (distCode >= 4 && distCode <= 5){
            dist = 5+GetBits(data,1)+(distCode-4)*std::pow(2,1);
          }
          else if (distCode >= 6 && distCode <= 7){
            dist = 9+GetBits(data,2)+(distCode-6)*std::pow(2,2);
          }
          else if (distCode >= 8 && distCode <= 9){
            dist = 17+GetBits(data,3)+(distCode-8)*std::pow(2,3);
          }
          else if (distCode >= 10 && distCode <= 11){
            dist = 33+GetBits(data,4)+(distCode-10)*std::pow(2,4);
          }
          else if (distCode >= 12 && distCode <= 13){
            dist = 65+GetBits(data,5)+(distCode-12)*std::pow(2,5);
          }
          else if (distCode >= 14 && distCode <= 15){
            dist = 129+GetBits(data,6)+(distCode-14)*std::pow(2,6);
          }
          else if (distCode >= 16 && distCode <= 17){
            dist = 257+GetBits(data,7)+(distCode-16)*std::pow(2,7);
          }
          else if (distCode >= 18 && distCode <= 19){
            dist = 513+GetBits(data,8)+(distCode-18)*std::pow(2,8);
          }
          else if (distCode >= 20 && distCode <= 21){
            dist = 1025+GetBits(data,9)+(distCode-20)*std::pow(2,9);
          }
          else if (distCode >= 22 && distCode <= 23){
            dist = 2049+GetBits(data,10)+(distCode-22)*std::pow(2,10);
          }
          else if (distCode >= 24 && distCode <= 25){
            dist = 4097+GetBits(data,11)+(distCode-24)*std::pow(2,11);
          }
          else if (distCode >= 26 && distCode <= 27){
            dist = 8193+GetBits(data,12)+(distCode-26)*std::pow(2,12);
          }
          else if (distCode >= 28 && distCode <= 29){
            dist = 16385+GetBits(data,13)+(distCode-28)*std::pow(2,13);
          }
          int offset = deCompressedData.size()-dist;
          for (int i=0;i<len;i++){
            deCompressedData.push_back(deCompressedData[(offset+i)]);
          }
        }

      }

    }
    else{
      std::cout << "Unsupported compression: " << type << std::endl;
      return 0;
    }
  }

  //Filtering
  std::vector<uint8_t> outData;
  index = 0;
  for (int i=0;i<image.height;i++){
    int filter = deCompressedData[index++];
    uint8_t filteredByte = 0;
    for (int j=0;j<image.width*numChannels;j++){
      switch(filter){
          case 0:
            filteredByte = deCompressedData[index++];
            break;
          case 1:
            {
              uint8_t byte = deCompressedData[index++];
              uint8_t previousByte = 0;
              if (j >= numChannels)
                previousByte = outData[i*image.width*numChannels+j-numChannels];
              filteredByte = byte+previousByte;
              break;
            }
          case 2:
            {
              uint8_t byte = deCompressedData[index++];
              uint8_t previousByte = 0;
              if (i >= 1)
                previousByte = outData[(i-1)*image.width*numChannels+j];
              filteredByte = byte+previousByte;
              break;
            }
          case 3:
            {
              uint8_t byte = deCompressedData[index++];
              uint8_t leftByte = 0;
              uint8_t upByte = 0;
              int mean = 0;
              if (j >= numChannels)
                leftByte = outData[i*image.width*numChannels+j-numChannels];
              if (i >= 1)
                upByte = outData[(i-1)*image.width*numChannels+j];
              mean = (leftByte+upByte)/2;
              filteredByte = byte+mean;
              break;
            }
          case 4:
            {
              uint8_t byte = deCompressedData[index++];
              uint8_t leftByte = 0;
              uint8_t upByte = 0;
              uint8_t upLeftByte = 0;
              if (j >= numChannels)
                leftByte = outData[i*image.width*numChannels+j-numChannels];
              if (i >= 1)
                upByte = outData[(i-1)*image.width*numChannels+j];
              if (j>=numChannels && i>=1)
                upLeftByte = outData[(i-1)*image.width*numChannels+j-numChannels];
              int p = leftByte+upByte-upLeftByte;
              int pa = std::abs(p-leftByte);
              int pb = std::abs(p-upByte);
              int pc = std::abs(p-upLeftByte);
              uint8_t paeth;
              if (pa <= pb && pa <= pc)
                paeth = leftByte;
              else if (pb <= pc)
                paeth = upByte;
              else
                paeth = upLeftByte;
              filteredByte = byte+paeth;
              break;
            }
          default:
            std::cout << "Unsupported Filter: " << filter << std::endl;
            return 0;
      }
      outData.push_back(filteredByte);
    }
  }

  if (bitDepth != 8){
    std::cout << "Unsupported Bit Depth: " << bitDepth << std::endl;
    return 0;
  }
  if (outData.size() != image.width*image.height*numChannels){
    std::cout << "Wrong image size" << std::endl;
    return 0;
  }
  int ind = 0;
  for (int y=image.height-1;y>=0;y--)
    for (int x=0;x<image.width;x++){
      switch(colorType){
        case 2:
          image.data[(y*image.width+x)*4] = outData[ind++];
          image.data[(y*image.width+x)*4+1] = outData[ind++];
          image.data[(y*image.width+x)*4+2] = outData[ind++];
          image.data[(y*image.width+x)*4+3] = 255;
          break;
        case 6:
          image.data[(y*image.width+x)*4] = outData[ind++];
          image.data[(y*image.width+x)*4+1] = outData[ind++];
          image.data[(y*image.width+x)*4+2] = outData[ind++];
          if (outData[ind] != 255)image.hasTransparency = true;
          image.data[(y*image.width+x)*4+3] = outData[ind++];
          break;
        default:
          std::cout << "Unsupported Color Type: " << colorType << std::endl;
          return 0;
      }
    }
  return 1;
}
