/// image8bit - A simple image processing module.
///
/// This module is part of a programming project
/// for the course AED, DETI / UA.PT
///
/// You may freely use and modify this code, at your own risk,
/// as long as you give proper credit to the original and subsequent authors.
///
/// João Manuel Rodrigues <jmr@ua.pt>
/// 2013, 2023
// 
// Student authors (fill in below):
// NMec:  Name:
// 113324, Pedro Mariano
// 113398, Diogo Valverde
// 
// Date:
//24/11/2023

#include "image8bit.h"
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "instrumentation.h"
#include <time.h>

// The data structure
//
// An image is stored in a structure containing 3 fields:
// Two integers store the image width and height.
// The other field is a pointer to an array that stores the 8-bit gray
// level of each pixel in the image.  The pixel array is one-dimensional
// and corresponds to a "raster scan" of the image from left to right,
// top to bottom.
// For example, in a 100-pixel wide image (img->width == 100),
//   pixel position (x,y) = (33,0) is stored in img->pixel[33];
//   pixel position (x,y) = (22,1) is stored in img->pixel[122].
// 
// Clients should use images only through variables of type Image,
// which are pointers to the image structure, and should not access the
// structure fields directly.

// Maximum value you can store in a pixel (maximum maxval accepted)
const uint8 PixMax = 255;

// Internal structure for storing 8-bit graymap images
struct image {
  int width;
  int height;
  int maxval;   // maximum gray value (pixels with maxval are pure WHITE)
  uint8* pixel; // pixel data (a raster scan)
};


// This module follows "design-by-contract" principles.
// Read `Design-by-Contract.md` for more details.

/// Error handling functions

// In this module, only functions dealing with memory allocation or file
// (I/O) operations use defensive techniques.
// 
// When one of these functions fails, it signals this by returning an error
// value such as NULL or 0 (see function documentation), and sets an internal
// variable (errCause) to a string indicating the failure cause.
// The errno global variable thoroughly used in the standard library is
// carefully preserved and propagated, and clients can use it together with
// the ImageErrMsg() function to produce informative error messages.
// The use of the GNU standard library error() function is recommended for
// this purpose.
//
// Additional information:  man 3 errno;  man 3 error;

// Variable to preserve errno temporarily
static int errsave = 0;

// Error cause
static char* errCause;

/// Error cause.
/// After some other module function fails (and returns an error code),
/// calling this function retrieves an appropriate message describing the
/// failure cause.  This may be used together with global variable errno
/// to produce informative error messages (using error(), for instance).
///
/// After a successful operation, the result is not garanteed (it might be
/// the previous error cause).  It is not meant to be used in that situation!
char* ImageErrMsg() { ///
  return errCause;
}


// Defensive programming aids
//
// Proper defensive programming in C, which lacks an exception mechanism,
// generally leads to possibly long chains of function calls, error checking,
// cleanup code, and return statements:
//   if ( funA(x) == errorA ) { return errorX; }
//   if ( funB(x) == errorB ) { cleanupForA(); return errorY; }
//   if ( funC(x) == errorC ) { cleanupForB(); cleanupForA(); return errorZ; }
//
// Understanding such chains is difficult, and writing them is boring, messy
// and error-prone.  Programmers tend to overlook the intricate details,
// and end up producing unsafe and sometimes incorrect programs.
//
// In this module, we try to deal with these chains using a somewhat
// unorthodox technique.  It resorts to a very simple internal function
// (check) that is used to wrap the function calls and error tests, and chain
// them into a long Boolean expression that reflects the success of the entire
// operation:
//   success = 
//   check( funA(x) != error , "MsgFailA" ) &&
//   check( funB(x) != error , "MsgFailB" ) &&
//   check( funC(x) != error , "MsgFailC" ) ;
//   if (!success) {
//     conditionalCleanupCode();
//   }
//   return success;
// 
// When a function fails, the chain is interrupted, thanks to the
// short-circuit && operator, and execution jumps to the cleanup code.
// Meanwhile, check() set errCause to an appropriate message.
// 
// This technique has some legibility issues and is not always applicable,
// but it is quite concise, and concentrates cleanup code in a single place.
// 
// See example utilization in ImageLoad and ImageSave.
//
// (You are not required to use this in your code!)


// Check a condition and set errCause to failmsg in case of failure.
// This may be used to chain a sequence of operations and verify its success.
// Propagates the condition.
// Preserves global errno!
static int check(int condition, const char* failmsg) {
  errCause = (char*)(condition ? "" : failmsg);
  return condition;
}


/// Init Image library.  (Call once!)
/// Currently, simply calibrate instrumentation and set names of counters.
void ImageInit(void) { ///
  InstrCalibrate();
  InstrName[0] = "pixmem";  // InstrCount[0] will count pixel array acesses
  // Name other counters here...
  InstrName[1] = "imageCreateDestroy";
  InstrName[2] = "fileIO";
  InstrName[3] = "pixelModifications";
  InstrName[4] = "transformOps";
  InstrName[5] = "filterOps";
  InstrName[6] = "memAllocFailures";
}

// Macros to simplify accessing instrumentation counters:
#define PIXMEM InstrCount[0]
// Add more macros here...
#define IMG_CREATE_DESTROY   InstrCount[1]
#define FILE_IO              InstrCount[2]
#define PIXEL_MODIFICATIONS  InstrCount[3]
#define TRANSFORM_OPS        InstrCount[4]
#define FILTER_OPS           InstrCount[5]
#define MEM_ALLOC_FAILURES   InstrCount[6]


/// Image management functions

/// Create a new black image.
///   width, height : the dimensions of the new image.
///   maxval: the maximum gray level (corresponding to white).
/// Requires: width and height must be non-negative, maxval > 0.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageCreate(int width, int height, uint8 maxval) { ///
  assert (width >= 0);
  assert (height >= 0);
  assert (0 < maxval && maxval <= PixMax);
  // Insert your code here!
  Image img = (Image)malloc(sizeof(struct image)); //Reserva memória para a estrutura da imagem
  if (!img) {
    //Define a causa da falha e retorna NULL se a reserva de memória falhar
    errCause = "Memory allocation failed for image structure";
    return NULL;
  }
  //Inicia os atributos da imagem
  img->width = width;
  img->height = height;
  img->maxval = maxval;

  img->pixel = (uint8*)malloc(width * height * sizeof(uint8)); //Reserva memória para os dados dos pixels
  if (!img->pixel) {
    //Define a causa da falha e retorna NULL se a reserva de memória falhar
    MEM_ALLOC_FAILURES++; //Incrementa o contador de falhas
    free(img); // Liberta o espaço reservado na memória para os dados dos pixels
    errCause = "Memory allocation failed for pixel data";
    return NULL;
  }

  // Initialize the image to black (all pixels to zero)
  memset(img->pixel, 0, width * height * sizeof(uint8));
  IMG_CREATE_DESTROY++; //Incrementa o contador de gerenciamento de recursos
  return img; //Retorna a imagem 
}

/// Destroy the image pointed to by (*imgp).
///   imgp : address of an Image variable.
/// If (*imgp)==NULL, no operation is performed.
/// Ensures: (*imgp)==NULL.
/// Should never fail, and should preserve global errno/errCause.
void ImageDestroy(Image* imgp) { ///
  assert (imgp != NULL);
  // Insert your code here!
  if (*imgp) {
    free((*imgp)->pixel); // Free the pixel data
    free(*imgp);          // Free the image structure
    *imgp = NULL;         // Set the pointer to NULL
  }
  IMG_CREATE_DESTROY++; //Incrementa o contador de gerenciamento de recursos
}


/// PGM file operations

// See also:
// PGM format specification: http://netpbm.sourceforge.net/doc/pgm.html
// Match and skip 0 or more comment lines in file f.
// Comments start with a # and continue until the end-of-line, inclusive.
// Returns the number of comments skipped.
static int skipComments(FILE* f) {
  char c;
  int i = 0;
  while (fscanf(f, "#%*[^\n]%c", &c) == 1 && c == '\n') {
    i++;
  }
  return i;
}

/// Load a raw PGM file.
/// Only 8 bit PGM files are accepted.
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageLoad(const char* filename) { ///
  FILE_IO++; //Incrementa as operações I/O 
  int w, h;  //Largura e altura da imagem
  int maxval;
  char c;
  FILE* f = NULL; //Ponteiro do arquivo
  Image img = NULL; //Ponteiro da imagem

  int success = check( (f = fopen(filename, "rb")) != NULL, "Open failed" ) &&
  // Parse PGM header
  check( fscanf(f, "P%c ", &c) == 1 && c == '5' , "Invalid file format" ) &&
  skipComments(f) >= 0 &&
  check( fscanf(f, "%d ", &w) == 1 && w >= 0 , "Invalid width" ) &&
  skipComments(f) >= 0 &&
  check( fscanf(f, "%d ", &h) == 1 && h >= 0 , "Invalid height" ) &&
  skipComments(f) >= 0 &&
  check( fscanf(f, "%d", &maxval) == 1 && 0 < maxval && maxval <= (int)PixMax , "Invalid maxval" ) &&
  check( fscanf(f, "%c", &c) == 1 && isspace(c) , "Whitespace expected" ) &&
  // Allocate image
  (img = ImageCreate(w, h, (uint8)maxval)) != NULL &&
  // Read pixels
  check( fread(img->pixel, sizeof(uint8), w*h, f) == w*h , "Reading pixels" );
  PIXMEM += (unsigned long)(w*h);  // count pixel memory accesses

  // Cleanup
  if (!success) {
    errsave = errno;
    ImageDestroy(&img); //Destrói a imagem em caso de falha
    errno = errsave;
  }
  if (f != NULL) fclose(f); //Fecha o arquivo se não for NULL
  return img; //Retorna a imagem
}

/// Save image to PGM file.
/// On success, returns nonzero.
/// On failure, returns 0, errno/errCause are set appropriately, and
/// a partial and invalid file may be left in the system.
int ImageSave(Image img, const char* filename) { ///
  assert (img != NULL); //Garante que há uma imagem
  int w = img->width; //Largura da imagem
  int h = img->height; //Altura da imagem
  uint8 maxval = img->maxval; //Valor máximo 
  FILE* f = NULL; //Ponteiro de arquivo 

  int success =
  check( (f = fopen(filename, "wb")) != NULL, "Open failed" ) && //Abre o arquivo e verifica se há erros 
  check( fprintf(f, "P5\n%d %d\n%u\n", w, h, maxval) > 0, "Writing header failed" ) && //Escreve o cabeçalho
  check( fwrite(img->pixel, sizeof(uint8), w*h, f) == w*h, "Writing pixels failed" ); //Escreve os pixels
  PIXMEM += (unsigned long)(w*h);  // count pixel memory accesses

  // Cleanup
  if (f != NULL) fclose(f); //Fecha o arquivo se não for NULL
  return success; 
}


/// Information queries

/// These functions do not modify the image and never fail.

/// Get image width
int ImageWidth(Image img) { ///
  assert (img != NULL);
  return img->width;
}

/// Get image height
int ImageHeight(Image img) { ///
  assert (img != NULL);
  return img->height;
}

/// Get image maximum gray level
int ImageMaxval(Image img) { ///
  assert (img != NULL);
  return img->maxval;
}

/// Pixel stats
/// Find the minimum and maximum gray levels in image.
/// On return,
/// *min is set to the minimum gray level in the image,
/// *max is set to the maximum.
void ImageStats(Image img, uint8* min, uint8* max) { ///
  assert (img != NULL);
  // Insert your code here!
  *min = 255;  
  *max = 0;    
  //Percorre cada pixel
  for (int y = 0; y < img->height; y++) {
    for (int x = 0; x < img->width; x++) {
      uint8 pixel = img->pixel[y * img->width + x]; //Obtém o valor do pixel
      if (pixel < *min) {
        *min = pixel; //Atualiza o mínimo
      }
      if (pixel > *max) {
        *max = pixel; //Atualiza o máximo
      }
    }
  }
}

/// Check if pixel position (x,y) is inside img.
int ImageValidPos(Image img, int x, int y) { ///
  assert (img != NULL);
  return (0 <= x && x < img->width) && (0 <= y && y < img->height);
}

/// Check if rectangular area (x,y,w,h) is completely inside img.
int ImageValidRect(Image img, int x, int y, int w, int h) { ///
  assert (img != NULL);
  // Insert your code here!
  // Primeiro, verifica se o canto esquerdo superior (x, y) está dentro da imagem,
  // e então verifica se o canto direito inferior (x + w, y + h) também está.
  // Isso é feito comparando as coordenadas x e y com as dimensões da imagem
  return (0 <= x && x + w <= img->width) && (0 <= y && y + h <= img->height);
}


/// Pixel get & set operations

/// These are the primitive operations to access and modify a single pixel
/// in the image.
/// These are very simple, but fundamental operations, which may be used to 
/// implement more complex operations.

// Transform (x, y) coords into linear pixel index.
// This internal function is used in ImageGetPixel / ImageSetPixel. 
// The returned index must satisfy (0 <= index < img->width*img->height)
static inline int G(Image img, int x, int y) {
  int index = y * img->width + x;
  // Insert your code here!
  // Verifica se o índice calculado está dentro dos limites válidos da imagem.
  // Esta afirmação garante que o índice não seja negativo e que esteja dentro
  // do intervalo total de pixels da imagem (largura * altura)
  assert (0 <= index && index < img->width*img->height);
  return index; //Devolve o índice calculado
}

/// Get the pixel (level) at position (x,y).
uint8 ImageGetPixel(Image img, int x, int y) { ///
  assert (img != NULL);
  assert (ImageValidPos(img, x, y));
  PIXMEM += 1;  // count one pixel access (read)
  return img->pixel[G(img, x, y)];
} 

/// Set the pixel at position (x,y) to new level.
void ImageSetPixel(Image img, int x, int y, uint8 level) { ///
  PIXEL_MODIFICATIONS++;
  assert (img != NULL);
  assert (ImageValidPos(img, x, y));
  PIXMEM += 1;  // count one pixel access (store)
  img->pixel[G(img, x, y)] = level;
} 


/// Pixel transformations

/// These functions modify the pixel levels in an image, but do not change
/// pixel positions or image geometry in any way.
/// All of these functions modify the image in-place: no allocation involved.
/// They never fail.


/// Transform image to negative image.
/// This transforms dark pixels to light pixels and vice-versa,
/// resulting in a "photographic negative" effect.
void ImageNegative(Image img) { ///
  assert (img != NULL);
  // Insert your code here!
  int totalPixels = img->width * img->height; //Calcula os pixels da imagem 
  for (int i = 0; i < totalPixels; i++) {
    img->pixel[i] = 255 - img->pixel[i];  // Assuming 8-bit gray levels
  }
}

/// Apply threshold to image.
/// Transform all pixels with level<thr to black (0) and
/// all pixels with level>=thr to white (maxval).
void ImageThreshold(Image img, uint8 thr) { ///
  assert (img != NULL);
  // Insert your code here!
  int totalPixels = img->width * img->height; //Calcula os pixels da imagem 
  for (int i = 0; i < totalPixels; i++) {
    // Define o pixel como preto ou branco com base no limiar 'thr'
    if (img->pixel[i] < thr) {
      img->pixel[i] = 0;  // Black
    } else {
      img->pixel[i] = 255;  // White
    }
  }
}

/// Brighten image by a factor.
/// Multiply each pixel level by a factor, but saturate at maxval.
/// This will brighten the image if factor>1.0 and
/// darken the image if factor<1.0.
void ImageBrighten(Image img, double factor) { ///
  assert(img != NULL && factor >= 0.0);
  // ? assert (factor >= 0.0);
  // Insert your code here!
  int totalPixels = img->width * img->height; //Calcula os pixels da imagem 
  for (int i = 0; i < totalPixels; i++) {
    int newLevel = (int)(img->pixel[i] * factor + 0.5); // Add 0.5 for rounding
    if (newLevel > img->maxval) newLevel = img->maxval; // Use img->maxval for saturation
    img->pixel[i] = (uint8)newLevel; // Atualiza o pixel com o novo nível de luminosidade
  }
}


/// Geometric transformations

/// These functions apply geometric transformations to an image,
/// returning a new image as a result.
/// 
/// Success and failure are treated as in ImageCreate:
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.

// Implementation hint: 
// Call ImageCreate whenever you need a new image!

/// Rotate an image.
/// Returns a rotated version of the image.
/// The rotation is 90 degrees anti-clockwise.
/// Ensures: The original img is not modified.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageRotate(Image img) { ///
  TRANSFORM_OPS++; //Incrementa o contador de operações de transformação
  assert (img != NULL);
  // Insert your code here!
  Image newImg = ImageCreate(img->height, img->width, img->maxval); //Cria nova imagem com as medidas invertidas
  if (newImg == NULL) return NULL;

  for (int x = 0; x < img->width; x++) {
    for (int y = 0; y < img->height; y++) {
      uint8 pixel = img->pixel[y * img->width + x];
      //Calcula novas coordenadas após rotação
      int newX = y;
      int newY = img->width - 1 - x;
      newImg->pixel[newY * newImg->width + newX] = pixel; //Atribui pixel à nova imagem
    }
  }

  return newImg; //Devolve a nova imagem 
}

/// Mirror an image = flip left-right.
/// Returns a mirrored version of the image.
/// Ensures: The original img is not modified.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageMirror(Image img) { ///
  assert (img != NULL);
  // Insert your code here!
  Image newImg = ImageCreate(img->width, img->height, img->maxval); //Cria imagem com as mesmas dimensões do original
  if (newImg == NULL) return NULL;

  for (int x = 0; x < img->width; x++) {
    for (int y = 0; y < img->height; y++) {
      uint8 pixel = img->pixel[y * img->width + x];
      int newX = img->width - 1 - x; //Calcula as novas coordenadas após o espelhamento 
      newImg->pixel[y * newImg->width + newX] = pixel; //Atribui o pixel à nova imagem 
    }
  }

  return newImg; //Devolve a nova imagem
}



/// Crop a rectangular subimage from img.
/// The rectangle is specified by the top left corner coords (x, y) and
/// width w and height h.
/// Requires:
///   The rectangle must be inside the original image.
/// Ensures:
///   The original img is not modified.
///   The returned image has width w and height h.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageCrop(Image img, int x, int y, int w, int h) { ///
  assert (img != NULL);
  assert (ImageValidRect(img, x, y, w, h));
  // Insert your code here!
  Image croppedImg = ImageCreate(w, h, img->maxval); //Cria nova imagem
  if (croppedImg == NULL) return NULL;

  for (int i = 0; i < h; ++i) {
    for (int j = 0; j < w; ++j) {
      croppedImg->pixel[i * w + j] = img->pixel[(y + i) * img->width + (x + j)]; //Copia os pixels da área específica para a nova imagem 
    }
  }

  return croppedImg; //Devolve a nova imagem
}


/// Operations on two images

/// Paste an image into a larger image.
/// Paste img2 into position (x, y) of img1.
/// This modifies img1 in-place: no allocation involved.
/// Requires: img2 must fit inside img1 at position (x, y).
void ImagePaste(Image img1, int x, int y, Image img2) { ///
  assert (img1 != NULL);
  assert (img2 != NULL);
  assert (ImageValidRect(img1, x, y, img2->width, img2->height));
  // Insert your code here!
  // Copia os pixels de img2 para img1 
  for (int i = 0; i < img2->height; ++i) {
    for (int j = 0; j < img2->width; ++j) {
      img1->pixel[(y + i) * img1->width + (x + j)] = img2->pixel[i * img2->width + j];
    }
  }
}

/// Blend an image into a larger image.
/// Blend img2 into position (x, y) of img1.
/// This modifies img1 in-place: no allocation involved.
/// Requires: img2 must fit inside img1 at position (x, y).
/// alpha usually is in [0.0, 1.0], but values outside that interval
/// may provide interesting effects.  Over/underflows should saturate.
void ImageBlend(Image img1, int x, int y, Image img2, double alpha) { ///
  assert(img1 != NULL && img2 != NULL); // Verifica se as imagens não são nulas
  assert(ImageValidRect(img1, x, y, img2->width, img2->height)); //Verifica se a opsição é válida
  assert(alpha >= 0.0 && alpha <= 1.0); // Verifica se o alpha está no intervalo
  // Junta os pixels de img2 em img1 usando o fator alpha
  for (int i = 0; i < img2->height; ++i) {
    for (int j = 0; j < img2->width; ++j) {
      int idx1 = (y + i) * img1->width + (x + j);
      int idx2 = i * img2->width + j;
      // Calcula o valor do combinado e garante a saturação
      double blendedValue = alpha * img2->pixel[idx2] + (1 - alpha) * img1->pixel[idx1];
      img1->pixel[idx1] = (uint8)(blendedValue > 255.0 ? 255.0 : blendedValue + 0.5);
    }
  }
}

/// Compare an image to a subimage of a larger image.
/// Returns 1 (true) if img2 matches subimage of img1 at pos (x, y).
/// Returns 0, otherwise.
int ImageMatchSubImage(Image img1, int x, int y, Image img2) { ///
  assert (img1 != NULL);
  assert (img2 != NULL);
  assert (ImageValidPos(img1, x, y));
  // Insert your code here!
  if (x + img2->width > img1->width || y + img2->height > img1->height) { // Verifica se img2 cabe em img1 
    return 0; 
  }
  // Compara cada pixel de img2 com a imagem correspondente em img1
  for (int i = 0; i < img2->height; ++i) {
    for (int j = 0; j < img2->width; ++j) {
      if (img1->pixel[(y + i) * img1->width + (x + j)] != img2->pixel[i * img2->width + j]) {
        return 0; 
      }
    }
  }

  return 1; // Se for tudo igual devolve verdadeiro
}

/// Locate a subimage inside another image.
/// Searches for img2 inside img1.
/// If a match is found, returns 1 and matching position is set in vars (*px, *py).
/// If no match is found, returns 0 and (*px, *py) are left untouched.
int ImageLocateSubImage(Image img1, int* px, int* py, Image img2, int* num_comparisons) { ///
  assert (img1 != NULL);
  assert (img2 != NULL);
  // Insert your code here!
  // Verifica se os ponteiros não são nulos
  assert(px != NULL);
  assert(py != NULL);
  assert(num_comparisons != NULL);
  *num_comparisons = 0; // Inicializa o contador de comparações
  // Tenta encontrar uma correspondência de img2 em img1
  for (int y = 0; y <= img1->height - img2->height; y++) {
    for (int x = 0; x <= img1->width - img2->width; x++) {
      (*num_comparisons)++; // Incrementa o contador de comparações
      if (ImageMatchSubImage(img1, x, y, img2)) { // Usa a função ImageMatchSubImage para ver se img2 correponde a uma subimagem de img1
        *px = x;
        *py = y;
        return 1; // Encontrou correspondência
      }
    }
  }

  return 0; // Correspondência não foi encontrada
}


/// Filtering

/// Blur an image by a applying a (2dx+1)x(2dy+1) mean filter.
/// Each pixel is substituted by the mean of the pixels in the rectangle
/// [x-dx, x+dx]x[y-dy, y+dy].
/// The image is changed in-place.
void ImageBlur(Image img, int dx, int dy) { ///
  // Insert your code here!
  assert(img != NULL); // Verifica se existe imagem 
  FILTER_OPS++; // Incrementa o contador de operações de filtragem
  // Armazena as dimensões da imagem 
  int width = img->width;
  int height = img->height;
  uint8* tempPixels = (uint8*)malloc(width * height * sizeof(uint8)); // Destina um buffer temporário para os pixels
  if (!tempPixels) {
    return; // Se falhar devolve sem fazer alterações
  }

  // Copia os pixels originais para um buffer temporário
  memcpy(tempPixels, img->pixel, width * height * sizeof(uint8));

  // Aplica o desfoque a cada pixel
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      double sum = 0;
      int count = 0;

      // Calcula a média para os pixels à volta
      for (int i = -dy; i <= dy; ++i) {
        for (int j = -dx; j <= dx; ++j) {
          int newY = y + i;
          int newX = x + j;

          // Verifica se as novas coordenadas estão dentro dos limites da imagem
          if (newY >= 0 && newY < height && newX >= 0 && newX < width) {
            sum += tempPixels[newY * width + newX];
            count++;
          }
        }
      }

      // Atualiza o valor do pixel com a média
      img->pixel[y * width + x] = (uint8)(sum / count + 0.5); // Adding 0.5 for rounding
    }
  }

  // Esvazia com o buffer temporário
  free(tempPixels);
}
void ImageFree(Image img) {
  if (img != NULL) {
    if (img->pixel != NULL) {
        free(img->pixel);
    }

    free(img);
  }
}


