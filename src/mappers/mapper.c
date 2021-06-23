#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mapper.h"
#include "utils.h"

static void select_mapper(Mapper*  mapper);
static void set_mapping(Mapper* mapper, uint16_t tr, uint16_t tl, uint16_t br, uint16_t bl);


static void select_mapper(Mapper* mapper){
    switch (mapper->mapper_num) {
        case NROM:
            load_NROM(mapper);
            break;
        case UXROM:
            load_UXROM(mapper);
            break;
        case MMC1:
            load_MMC1(mapper);
            break;
        case CNROM:
        case MMC3:
        default:
            LOG(ERROR, "Mapper no %u not implemented", mapper->mapper_num);
            exit(EXIT_FAILURE);
    }
}


static void set_mapping(Mapper* mapper, uint16_t tr, uint16_t tl, uint16_t br, uint16_t bl){
    mapper->name_table_map[0] = tr;
    mapper->name_table_map[1] = tl;
    mapper->name_table_map[2] = br;
    mapper->name_table_map[3] = bl;
}


void set_mirroring(Mapper* mapper, Mirroring mirroring){
    switch (mirroring) {
        case HORIZONTAL:
            set_mapping(mapper, 0, 0, 0x400, 0x400);
            LOG(DEBUG, "Using mirroring: Horizontal");
            break;
        case VERTICAL:
            set_mapping(mapper,0, 0x400, 0, 0x400);
            LOG(DEBUG, "Using mirroring: Vertical");
            break;
        case ONE_SCREEN_LOWER:
        case ONE_SCREEN:
            set_mapping(mapper,0, 0, 0, 0);
            LOG(DEBUG, "Using mirroring: Single screen lower");
            break;
        case ONE_SCREEN_UPPER:
            set_mapping(mapper, 0x400, 0x400, 0x400, 0x400);
            LOG(DEBUG, "Using mirroring: Single screen upper");
            break;
        default:
            set_mapping(mapper,0, 0, 0, 0);
            LOG(ERROR, "Unknown mirroring %u", mirroring);
    }
}


void load_file(char* file_name, Mapper* mapper){
    FILE* file = fopen(file_name, "rb");

    if(file == NULL){
        LOG(ERROR, "file '%s' not found", file_name);
        exit(EXIT_FAILURE);
    }

    char header_start[5];
    fread(header_start, 1, 4, file);
    header_start[4] = '\0';

    if(strncmp(header_start, "NES\x1A", 4) != 0){
        LOG(ERROR, "unknown file format");
        exit(EXIT_FAILURE);
    }

    fread(&mapper->PRG_banks, 1, 1, file);
    fread(&mapper->CHR_banks, 1, 1, file);

    LOG(INFO, "PRG banks (16KB): %u", mapper->PRG_banks);
    LOG(INFO, "CHR banks (8KB): %u", mapper->CHR_banks);

    uint8_t ROM_ctrl1, ROM_ctrl2;

    fread(&ROM_ctrl1, 1, 1, file);

    if(ROM_ctrl1 & BIT_1){
        mapper->save_RAM = malloc(0x2000);
        LOG(INFO, "Battery backed save RAM 8KB : Available");
    }

    if(ROM_ctrl1 & BIT_2) {
        LOG(ERROR, "Trainer not supported");
        exit(EXIT_FAILURE);
    }

    if(ROM_ctrl1 & BIT_3){
        mapper->mirroring = FOUR_SCREEN;
    }
    else if(ROM_ctrl1 & BIT_0) {
        mapper->mirroring = VERTICAL;
    }
    else {
        mapper->mirroring = HORIZONTAL;
    }

    mapper->mapper_num = (ROM_ctrl1 & 0xF0) >> 4;
    fread(&ROM_ctrl2, 1, 1, file);
    mapper->mapper_num |= ROM_ctrl2 & 0xF0;

    LOG(INFO, "Using mapper #%d", mapper->mapper_num);

    fread(&mapper->RAM_banks, 1, 1, file);

    if(mapper->RAM_banks == 0)
        mapper->RAM_banks++;  // assume 1 when set to zero
    LOG(INFO, "RAM Banks (2kb): %u", mapper->RAM_banks);

    fseek(file, 7, SEEK_CUR);

    mapper->PRG_ROM = malloc(0x4000 * mapper->PRG_banks);
    fread(mapper->PRG_ROM, 1, 0x4000 * mapper->PRG_banks, file);

    if(mapper->CHR_banks) {
        mapper->CHR_RAM = malloc(0x2000 * mapper->CHR_banks);
        fread(mapper->CHR_RAM, 1, 0x2000 * mapper->CHR_banks, file);
        LOG(INFO, "CHR-ROM : Available");
    }

    fclose(file);

    select_mapper(mapper);
    set_mirroring(mapper, mapper->mirroring);
}

void free_mapper(Mapper* mapper){
    if(mapper->PRG_ROM != NULL)
        free(mapper->PRG_ROM);
    if(mapper->CHR_RAM != NULL)
        free(mapper->CHR_RAM);
    if(mapper->extension != NULL)
        free(mapper->extension);
}