import os
import glob
import cv2


with open("FontMap.hpp", 'w') as outFile:
  outFile.write('#include <array>\n#include <unordered_map>\n')
  outFile.write('\nnamespace dl::font\n{\n  const std::unordered_map<char, ' \
    'std::array<uint8_t, 112>& GetFontMap()\n  {\n')

  First = True
  outFile.write('    static std::array<uint8_t, 112> FontMap\n    {\n')
  filenames =  glob.glob('*.png')
  filenames.sort()
  for filename in filenames:
    image = cv2.imread(filename)
    image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    outFile.write('      {\n')
    outFile.write("        {'"+filename[0] + "'},\n")
    outFile.write('        {\n')
    for i in range(image.shape[0]):
      outFile.write('          ')
      for j in range(image.shape[1]):
        outFile.write('{:3d}u,'.format(image[i][j]))
      outFile.write('\n')
    outFile.write('        }') # end matrix
    outFile.write('\n      }\n')
  outFile.write('    };\n\n    return FontMap;')
  outFile.write('\n  }')
  outFile.write('\n}')


