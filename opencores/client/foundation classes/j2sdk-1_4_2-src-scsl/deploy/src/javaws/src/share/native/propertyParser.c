/*
 * @(#)propertyParser.c	1.21 03/04/18
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "system.h"
#include "util.h"
#include "propertyParser.h"
#include "configurationFile.h"

/* Local methods definitions */
static char* GetNextOption(char* s, char** option, char **value);

int GetJREIndex(char *key) {
  char *keydup = strdup(key);
  unsigned int subkeylen = strlen(CFG_JRE_KEY);
  int index = -1;
  
  if (strlen(keydup) >= subkeylen && 
      strncmp(keydup, CFG_JRE_KEY, subkeylen)==0) {
    char *p = &(keydup[subkeylen]);
    if (p) {
      char *q = strchr(p, '.');
      if (q) {
	*q = 0;
	index = atoi(p);
      }
    }
  }
  free(keydup);
  return index;
}

/* path must be an absolute path, and must not have FILE_SEPARATOR at end */
void recursive_create_directory(char *path) {
  char *p, *dir;
  struct stat stat_buf;

  /* if path already exists then return */
  if (stat(path, &stat_buf)==0) return;

  /* find previous file separator char from end of string */
  p = strrchr(path, FILE_SEPARATOR);
  if (p == NULL) return;

  /* store before chopping */
  dir = strdup(path);

  /* chop string at file separator char */
  if (p > path && *(p-1) != ':') *p = 0;
  else *(p+1) = 0;

  /* recurse */
  recursive_create_directory(path);

  sysCreateDirectory(dir);

  free(dir);
}


/*
 * At this point the property list looks like this :
 *
 *       entry <-- userHead
 *         |
 *         V
 *       entry
 *         |
 *         V
 *     (more entries)
 *         |
 *         V
 *       entry <-- installHead
 *         |
 *         V
 *     (more entries)
 *         |
 *         V
 *        NULL
 *
 * We want to store everything between userHead and installHead (i.e. user
 * specific properties) in the user properties file.
 */
void storePropertyFile(char *filename, PropertyFileEntry* userHead,
		       PropertyFileEntry* installHead) {
  PropertyFileEntry *entry;
  FILE *fp = NULL;
  int index;
  char buf[4096], *dir, *p;

  /* create directories if they don't exist */
  dir = strdup(filename);
  p = strrchr(dir, FILE_SEPARATOR);
  if (p != NULL) {
    if (p > dir && *(p-1) != ':') *p = 0;
    else *(p+1) = 0;
    recursive_create_directory(dir);
  }
  free(dir);

  fp = fopen(filename, "wb");
  if (!fp) return;

  for (entry = userHead;
       entry != NULL && entry != installHead; 
       entry = entry->next) {
    if ((index = GetJREIndex(entry->key)) == -1 ||
	isJRERegistered(index) == 0 ||
	isJREConfirmed(index) == 1) {
      /* if it's a regular (non JRE) entry, or an unregistered JRE, or a
	 confirmed JRE, write it to the file */
      if (entry->key != NULL && entry->value != NULL) {
	int i = 0;
	char *p = entry->key;
	char *output = NULL;
	while (*p != 0) {
	  if (*p == '\\') {
	    buf[i++] = '\\';
	  }
	  buf[i++] = *p++;
	}
	buf[i++] = '=';
	p = entry->value;
	while (*p != 0) {
	  if (*p == '\\') {
	    buf[i++] = '\\';
	  }
	  buf[i++] = *p++;
	}
	buf[i] = 0;
	strcat(buf, "\n");
	output = sysMBCSToSeqUnicode(buf);
	fwrite(output, 1, strlen(output), fp);
	free(output);
      }
    }
  }
  fclose(fp);
}


/*
 * Parses a property file. Returns NULL if file not found or it is empty 
 */
PropertyFileEntry* parsePropertyFile(char *filename, PropertyFileEntry* head) {
    PropertyFileEntry* entry = NULL;
    char* buffer;
    int size;

    /* Read contents of file into memory */
    size  = ReadFileToBuffer(filename, &buffer);
    /* File not found? */
    if (buffer == NULL) return head;
    
    entry = parsePropertyStream(buffer, head);

    free(buffer);
    return entry;        
}

/*
 * Parses a string buffer as a property file
 */
PropertyFileEntry* parsePropertyStream(char *s, PropertyFileEntry* head) {
    PropertyFileEntry* entry = NULL;
    char* key;
    char* value;

    s = GetNextOption(s, &key, &value);
    while(s != NULL) {
        /* Construct new entry element */
        PropertyFileEntry* entry = (PropertyFileEntry*)malloc(sizeof(PropertyFileEntry));
        entry->key   = key;
        entry->value = value;
	/* record JRE index if this is a JRE */
	if (key != NULL) {
	  int index;
	  if ((index = GetJREIndex(key)) != -1) {
	    addToIndexArray(index);
	  }
	}   
        entry->next  = head;
        head = entry;
        /* Parse next entry */
        s = GetNextOption(s, &key, &value);
    }        
    return head;
}

/* This interates through a property file, returning one
 * (option,value) pair at a time. It automatically skips
 * comments and blank lines. Returns true if a pair is
 * found, false when at the end of the file.
 *
 * The file is parsed pretty much according to the spec.
 * for the java.util.Properties output format. However,
 * multiline values and unicode escaped values are not
 * supported. (This should be removed/converted in a pre-parse)
 */
static char* GetNextOption(char* p, char** optionPtr, char **valuePtr) {
    char* mark;
    char* str;
    char* end;
    int len;
    
    *optionPtr = NULL;
    *valuePtr  = NULL;

    /* Check if we are at the end */
    if (p == NULL || *p == '\0') return NULL;

    /* Skip whitespace, newlines, and comments */
    do {
        mark = p;
        while(iswspace(*p) || *p == '\r' || *p == '\n') p++;
        if (*p == '#') {
            p++;
            while(*p && (*p != '\n' && *p != '\r')) p++;
        }
    } while(mark != p);    

    /* Are at end of the buffer? */
    if (!(*p)) return NULL;

    /* Parse key */
    mark = p;
    
    /* Find end of option */
    while(*p && (!(iswspace(*p) || *p == ':' || *p == '=')) || 
		 ((mark != p) && (*(p-1) == '\\'))) p++;

    /* Allocate memory for key */
    len = p - mark;

    str = (char*)malloc(len + 1);
    strncpy(str, mark, len);
    str[len] = '\0';
    *optionPtr = str;

    /* Skip until start of value */
    while(iswspace(*p)) p++;
    if (*p && (*p == ':' || *p == '=')) {
        p++;
	while(iswspace(*p) && *p != '\n' && *p != '\r') p++;
    }
    /* End of stream? */
    if (!(*p)) return NULL;

    /* Find end of value */
    mark = p;
    while(*p && *p != '\n' && *p != '\r') p++;

    /* Trim trailing whitespaces */
    end = p;
    while(end > mark && iswspace(end[-1])) end--;

    /** Allocate buffer for value */
    len = end - mark;
    str = (char*)malloc(len + 1);
    strncpy(str, mark, len);
    str[len] = '\0';
    *valuePtr = str;
    
    /* Handle potential escape characters in value */
    mark = str;
    while (*str) {
        if (*str != '\\' || str[1] == 'u') {
	    *mark = *str;
	    mark++; str++;
        } else {
            switch(*(++str)) {
                case 't': *mark = '\t'; break;
                case 'n': *mark = '\n'; break;
                case 'r': *mark = '\r'; break;
                default : *mark = *str;
            }
            if (*str) str++;                    
            mark++;
        }
    }
    *mark = '\0';

    return p;
}

/* Find a specific entry */
char* GetPropertyValue(PropertyFileEntry* head, char* key) {
    char *match = NULL;

    /* search through the entire list because later entries should override */
    /* earlier entries */
    while(head != NULL) {
        if (strcmp(head->key, key) == 0) {
            match = head->value;
        }
        head = head->next;
    }        
    return match;
}




/* Release all memory used by the property entry */
void FreePropertyEntry(PropertyFileEntry* head) {
    PropertyFileEntry* next = NULL;

    while(head != NULL) {
        next = head->next;
        free(head->key);
        free(head->value);
        free(head);
        head = next;        
    }        
}

void PrintPropertyEntry(PropertyFileEntry* entry) {
    if (entry == NULL) {
        printf("---end---\n");
    } else {
        printf("PROP (%s, %s)\n", entry->key, entry->value);
        PrintPropertyEntry(entry->next);
    }
}
