if(NOT EXISTS "${TERRAIN_SOURCE}")
    message(FATAL_ERROR "Terrain source missing: ${TERRAIN_SOURCE}")
endif()

file(READ "${TERRAIN_SOURCE}" terrain_source)

if(NOT terrain_source MATCHES "BITMAP_t\\* b = &Bitmaps\\[BITMAP_MAPTILE \\+ Texture\\]")
    message(FATAL_ERROR "Terrain UV generation must use the selected texture metadata")
endif()
if(NOT terrain_source MATCHES "Width = 64\\.f / b->Width" OR
   NOT terrain_source MATCHES "Height = 64\\.f / b->Height")
    message(FATAL_ERROR "Terrain tile UVs must derive both axes from each texture's dimensions")
endif()
if(NOT terrain_source MATCHES "FaceTexture\\(Texture, xf, yf, Water, false\\);[ \t\r\n]*RenderFace\\(")
    message(FATAL_ERROR "Base terrain draws must generate UVs for their selected texture")
endif()
if(NOT terrain_source MATCHES "TerrainMappingLayer2\\[TerrainIndex1\\][^}]*FaceTexture\\(Texture, xf, yf, Water, false\\);[ \t\r\n]*RenderFaceAlpha\\(")
    message(FATAL_ERROR "Overlay terrain draws must regenerate UVs for their selected texture")
endif()
