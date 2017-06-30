/*
 *   
 *   Ophcrack is a Lanmanager/NTLM hash cracker based on the faster time-memory
 *   trade-off using rainbow tables. 
 *   
 *   Created with the help of: Maxime Mueller, Luca Wullschleger, Claude
 *   Hochreutiner, Andreas Huber and Etienne Dysli.
 *   
 *   Copyright (c) 2013 Philippe Oechslin, Cedric Tissieres, 
 *                      Bertrand Mesot, Pierre Lestringant
 *   
 *   Ophcrack is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *   
 *   Ophcrack is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *   
 *   You should have received a copy of the GNU General Public License
 *   along with Ophcrack; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *   
 *   This program is released under the GPL with the additional exemption 
 *   that compiling, linking, and/or using OpenSSL is allowed.
 *   
 *   
 *   
 *   
*/
/* WARNING: this file is a NOT copy from implCPU/v6 */

#include <stdlib.h>
#include <stdio.h>
#include <expat.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "ioPattern.h"
#include "aliasPattern.h"
#include "ioMarkov.h"
#include "patternDescriptor.h"

typedef struct xmlDataWrapper xmlDataWrapper;
struct xmlDataWrapper{
  patternArray* array;
  unsigned int currentPattern;
  unsigned int currentCharType;
};

void start_element(void *data, const char *element, const char **attribute){
  xmlDataWrapper* wrapper = (xmlDataWrapper*)data;
  unsigned int i;
	
  if (!strcmp(element, "patternArray")){
    wrapper->array->size = atoi(attribute[1]);
    wrapper->array->sizeArray = (uint64_t*)malloc(sizeof(uint64_t)*wrapper->array->size);
    wrapper->array->patternDescriptor = (uint64_t*)malloc(sizeof(uint64_t)*wrapper->array->size);
    wrapper->array->patternLength = (uint64_t*)malloc(sizeof(uint64_t)*wrapper->array->size);
    wrapper->array->lut = (lookUpTable*)malloc(sizeof(lookUpTable));
		
    memset(wrapper->array->patternDescriptor, 0, wrapper->array->size*sizeof(uint64_t));
    memset(wrapper->array->patternLength, 0, wrapper->array->size*sizeof(uint64_t));
		
    wrapper->currentPattern = 0;
    return;
  }
  if (!strcmp(element, "pattern")){
    wrapper->array->sizeArray[wrapper->currentPattern] = strtoull(attribute[1], NULL, 16);
    wrapper->currentCharType = 0;
    return;
  }
  if (!strcmp(element, "element")){
    switch(attribute[3][0]){
    case 'U' : {
      switch(attribute[3][1]){
      case 'N' : {
	for (i = 0; i < (unsigned int)atoi(attribute[1]); i++){
	  wrapper->array->patternDescriptor[wrapper->currentPattern] |= (uint64_t)DESC_UPPER_NOMARK<< 4*(wrapper->currentCharType + i);
	}
	wrapper->currentCharType += atoi(attribute[1]);
	break;
      }
      case '1' : {
	wrapper->array->patternDescriptor[wrapper->currentPattern] |= (uint64_t)DESC_UPPER_MARKFIRST<< 4*wrapper->currentCharType ;
	wrapper->array->patternLength[wrapper->currentPattern] |= (uint64_t)((atoi(attribute[1])-1) & 0x0000000f)<< 4*wrapper->currentCharType ;
	wrapper->currentCharType ++;
	break;
      }
      case '2' : {
	wrapper->array->patternDescriptor[wrapper->currentPattern] |= (uint64_t)DESC_UPPER_MARKSECOND<< 4*wrapper->currentCharType ;
	wrapper->currentCharType ++;
	break;
      }
      case 'M' : {
	for (i = 0; i < (unsigned int)atoi(attribute[1]); i++){
	  wrapper->array->patternDescriptor[wrapper->currentPattern] |= (uint64_t)DESC_UPPER_MARK<< 4*(wrapper->currentCharType + i);
	}
	wrapper->currentCharType += atoi(attribute[1]);
	break;
      }
      default : {
	printf("Error while parsing XML file: unknown symbol\n");
	break;
      }
      }
      break;
    }
    case 'L' : {
      switch(attribute[3][1]){
      case 'N' : {
	for (i = 0; i < (unsigned int)atoi(attribute[1]); i++){
	  wrapper->array->patternDescriptor[wrapper->currentPattern] |= (uint64_t)DESC_LOWER_NOMARK<< 4*(wrapper->currentCharType + i);
	}
	wrapper->currentCharType += atoi(attribute[1]);
	break;
      }
      case '1' : {
	wrapper->array->patternDescriptor[wrapper->currentPattern] |= (uint64_t)DESC_LOWER_MARKFIRST<< 4*wrapper->currentCharType ;
	wrapper->array->patternLength[wrapper->currentPattern] |= (uint64_t)((atoi(attribute[1])-1) & 0x0000000f)<< 4*wrapper->currentCharType ;
	wrapper->currentCharType ++;
	break;
      }
      case '2' : {
	wrapper->array->patternDescriptor[wrapper->currentPattern] |= (uint64_t)DESC_LOWER_MARKSECOND<< 4*wrapper->currentCharType ;
	wrapper->currentCharType ++;
	break;
      }
      case 'M' : {
	for (i = 0; i < (unsigned int)atoi(attribute[1]); i++){
	  wrapper->array->patternDescriptor[wrapper->currentPattern] |= (uint64_t)DESC_LOWER_MARK<< 4*(wrapper->currentCharType + i);
	}
	wrapper->currentCharType += atoi(attribute[1]);
	break;
      }
      default : {
	printf("Error while parsing XML file: unknown symbol\n");
	break;
      }
      }
      break;
    }
    case 'N' : {
      for (i = 0; i < (unsigned int)atoi(attribute[1]); i++){
	wrapper->array->patternDescriptor[wrapper->currentPattern] |= (uint64_t)DESC_NUMERAL << 4*(wrapper->currentCharType + i);
      }
      wrapper->currentCharType += atoi(attribute[1]);
      break;
    }
    case 'S' : {
      for (i = 0; i < (unsigned int)atoi(attribute[1]); i++){
	wrapper->array->patternDescriptor[wrapper->currentPattern] |= (uint64_t)DESC_SPECIAL << 4*(wrapper->currentCharType + i);
      }
      wrapper->currentCharType += atoi(attribute[1]);
      break;
    }
    default  : {
      printf("Error while parsing XML file: unknown symbol\n");
      break;
    }
    }
    return;
  }
}

void end_element(void *data, const char *element){
  xmlDataWrapper* wrapper = (xmlDataWrapper*)data;
  if (!strcmp(element, "pattern")){
    wrapper->currentPattern ++;
    return;
  }
}

char readPatternArrayFromXMLFile(patternArray* array, char* path, char* fileName){
  struct stat sb;
  char* xmlBuff;
  char buffer[256];
  int xmlFile;
	
  if (fileName == NULL){
    sprintf(buffer, "%s/%s", path, DEFAULT_PATTERN_FILE_NAME);
  }
  else{
    sprintf(buffer, "%s/%s", path, fileName);
  }
	
  xmlFile = open(buffer, O_RDONLY);
	
  if (xmlFile == -1){
    return 0;
  }
	
  if (fstat(xmlFile, &sb) == -1){
    close(xmlFile);
    return 0;
  }
	
  xmlBuff = mmap(NULL, sb.st_size,  PROT_READ, MAP_PRIVATE, xmlFile, 0);
  if (xmlBuff == MAP_FAILED){
    close(xmlFile);
    return 0;
  }
	
  xmlDataWrapper data;
  data.array = array;
  data.currentPattern = 0;
  data.currentCharType = 0;
	
	
  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, &data);
  XML_SetElementHandler(parser, start_element, end_element);
	
  if(XML_Parse(parser, xmlBuff, sb.st_size,  XML_TRUE) == XML_STATUS_ERROR){
    XML_ParserFree(parser);
    munmap(xmlBuff, sb.st_size);
    close(xmlFile);
    return 0;
  }
	
  initLookUpTable(array->lut, array);
		
  XML_ParserFree(parser);
  munmap(xmlBuff, sb.st_size);
  close(xmlFile);
	
  return 1;
}
