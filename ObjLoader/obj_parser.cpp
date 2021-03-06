#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "obj_parser.h"
#include "list.h"
#include "string_extra.h"
#include <Exception/UnknownCommandException.h>
#include <Exception/OpeningFileException.h>
#include <QString>
#include <QDebug>

#define WHITESPACE " \t\n\r"

void obj_free_half_list(list *listo)
{
	list_delete_all(listo);
	free(listo->names);

}

int obj_convert_to_list_index(int current_max, int index)
{
    if(index == 0)
		return -1;
		
    if(index < 0)
		return current_max + index;
		
    return index - 1;
}

void obj_convert_to_list_index_v(int current_max, int *indices)
{
	for(int i=0; i<MAX_VERTEX_COUNT; i++)
		indices[i] = obj_convert_to_list_index(current_max, indices[i]);
}

void obj_set_material_defaults(ObjMaterial *mtl)
{
	mtl->amb[0] = 0.2;
	mtl->amb[1] = 0.2;
	mtl->amb[2] = 0.2;
	mtl->diff[0] = 0.8;
	mtl->diff[1] = 0.8;
	mtl->diff[2] = 0.8;
	mtl->spec[0] = 1.0;
	mtl->spec[1] = 1.0;
	mtl->spec[2] = 1.0;
	mtl->reflect = 0.0;
	mtl->trans = 1;
	mtl->glossy = 98;
	mtl->shiny = 0;
	mtl->refract_index = 1;
	mtl->texture_filename[0] = '\0';
}

int obj_parse_vertex_index(int *vertex_index, int *texture_index, int *normal_index)
{
	char *temp_str;
	char *token;
	int vertex_count = 0;

	
	while( (token = strtok(NULL, WHITESPACE)) != NULL)
	{
		if(texture_index != NULL)
			texture_index[vertex_count] = 0;
		if(normal_index != NULL)
		normal_index[vertex_count] = 0;

		vertex_index[vertex_count] = atoi( token );
		
		if(contains(token, "//"))  //normal only
		{
			temp_str = strchr(token, '/');
			temp_str++;
			normal_index[vertex_count] = atoi( ++temp_str );
		}
		else if(contains(token, "/"))
		{
			temp_str = strchr(token, '/');
			texture_index[vertex_count] = atoi( ++temp_str );

			if(contains(temp_str, "/"))
			{
				temp_str = strchr(temp_str, '/');
				normal_index[vertex_count] = atoi( ++temp_str );
			}
		}
		
		vertex_count++;
	}

	return vertex_count;
}

ObjFace* obj_parse_face(ObjGrowableSceneData *scene)
{
	int vertex_count;
    ObjFace *face = (ObjFace*)malloc(sizeof(ObjFace));
	
	vertex_count = obj_parse_vertex_index(face->vertex_index, face->texture_index, face->normal_index);
	obj_convert_to_list_index_v(scene->vertex_list.item_count, face->vertex_index);
	obj_convert_to_list_index_v(scene->vertex_texture_list.item_count, face->texture_index);
	obj_convert_to_list_index_v(scene->vertex_normal_list.item_count, face->normal_index);
	face->vertex_count = vertex_count;

	return face;
}

ObjSphere* obj_parse_sphere(ObjGrowableSceneData *scene)
{
	int temp_indices[MAX_VERTEX_COUNT];

    ObjSphere *obj = (ObjSphere*)malloc(sizeof(ObjSphere));
	obj_parse_vertex_index(temp_indices, obj->texture_index, NULL);
	obj_convert_to_list_index_v(scene->vertex_texture_list.item_count, obj->texture_index);
	obj->pos_index = obj_convert_to_list_index(scene->vertex_list.item_count, temp_indices[0]);
	obj->up_normal_index = obj_convert_to_list_index(scene->vertex_normal_list.item_count, temp_indices[1]);
	obj->equator_normal_index = obj_convert_to_list_index(scene->vertex_normal_list.item_count, temp_indices[2]);

	return obj;
}

ObjPlane* obj_parse_plane(ObjGrowableSceneData *scene)
{
	int temp_indices[MAX_VERTEX_COUNT];

    ObjPlane *obj = (ObjPlane*)malloc(sizeof(ObjPlane));
	obj_parse_vertex_index(temp_indices, obj->texture_index, NULL);
	obj_convert_to_list_index_v(scene->vertex_texture_list.item_count, obj->texture_index);
	obj->pos_index = obj_convert_to_list_index(scene->vertex_list.item_count, temp_indices[0]);
	obj->normal_index = obj_convert_to_list_index(scene->vertex_normal_list.item_count, temp_indices[1]);
	obj->rotation_normal_index = obj_convert_to_list_index(scene->vertex_normal_list.item_count, temp_indices[2]);

	return obj;
}

ObjLightPoint* obj_parse_light_point(ObjGrowableSceneData *scene)
{
    ObjLightPoint *o= (ObjLightPoint*)malloc(sizeof(ObjLightPoint));
	o->pos_index = obj_convert_to_list_index(scene->vertex_list.item_count, atoi( strtok(NULL, WHITESPACE)) );
	return o;
}

ObjLightQuad* obj_parse_light_quad(ObjGrowableSceneData *scene)
{
    ObjLightQuad *o = (ObjLightQuad*)malloc(sizeof(ObjLightQuad));
	obj_parse_vertex_index(o->vertex_index, NULL, NULL);
	obj_convert_to_list_index_v(scene->vertex_list.item_count, o->vertex_index);

	return o;
}

ObjLightDisc* obj_parse_light_disc(ObjGrowableSceneData *scene)
{
	int temp_indices[MAX_VERTEX_COUNT];

    ObjLightDisc *obj = (ObjLightDisc*)malloc(sizeof(ObjLightDisc));
	obj_parse_vertex_index(temp_indices, NULL, NULL);
	obj->pos_index = obj_convert_to_list_index(scene->vertex_list.item_count, temp_indices[0]);
	obj->normal_index = obj_convert_to_list_index(scene->vertex_normal_list.item_count, temp_indices[1]);

	return obj;
}

ObjVector* obj_parse_vector()
{
    ObjVector *v = (ObjVector*)malloc(sizeof(ObjVector));
	v->e[0] = atof( strtok(NULL, WHITESPACE));
	v->e[1] = atof( strtok(NULL, WHITESPACE));
	v->e[2] = atof( strtok(NULL, WHITESPACE));
	return v;
}

void obj_parse_camera(ObjGrowableSceneData *scene, ObjCamera *camera)
{
	int indices[3];
	obj_parse_vertex_index(indices, NULL, NULL);
	camera->camera_pos_index = obj_convert_to_list_index(scene->vertex_list.item_count, indices[0]);
	camera->camera_look_point_index = obj_convert_to_list_index(scene->vertex_list.item_count, indices[1]);
	camera->camera_up_norm_index = obj_convert_to_list_index(scene->vertex_normal_list.item_count, indices[2]);
}

int obj_parse_mtl_file(char *filename, list *material_list)
{
	int line_number = 0;
	char *current_token;
	char current_line[OBJ_LINE_SIZE];
	char material_open = 0;
    ObjMaterial *current_mtl = NULL;
	FILE *mtl_file_stream;
	
    // открытие сцены
	mtl_file_stream = fopen( filename, "r");
	if(mtl_file_stream == 0)
	{
        //fprintf(stderr, "Error reading file: %s\n", filename);
		return 0;
	}
		
	list_make(material_list, 10, 1);

	while( fgets(current_line, OBJ_LINE_SIZE, mtl_file_stream) )
	{
		current_token = strtok( current_line, " \t\n\r");
		line_number++;
		
        // пропуск комментариев
		if( current_token == NULL || strequal(current_token, "//") || strequal(current_token, "#"))
			continue;
		

		else if( strequal(current_token, "newmtl"))
		{
			material_open = 1;
            current_mtl = (ObjMaterial*) malloc(sizeof(ObjMaterial));
			obj_set_material_defaults(current_mtl);
			
            // получение имени
			strncpy(current_mtl->name, strtok(NULL, " \t"), MATERIAL_NAME_SIZE);
			list_add_item(material_list, current_mtl, current_mtl->name);
		}
		
		else if( strequal(current_token, "Ka") && material_open)
		{
			current_mtl->amb[0] = atof( strtok(NULL, " \t"));
			current_mtl->amb[1] = atof( strtok(NULL, " \t"));
			current_mtl->amb[2] = atof( strtok(NULL, " \t"));
		}

		else if( strequal(current_token, "Kd") && material_open)
		{
			current_mtl->diff[0] = atof( strtok(NULL, " \t"));
			current_mtl->diff[1] = atof( strtok(NULL, " \t"));
			current_mtl->diff[2] = atof( strtok(NULL, " \t"));
		}
		
		else if( strequal(current_token, "Ks") && material_open)
		{
			current_mtl->spec[0] = atof( strtok(NULL, " \t"));
			current_mtl->spec[1] = atof( strtok(NULL, " \t"));
			current_mtl->spec[2] = atof( strtok(NULL, " \t"));
		}
        // яркость
		else if( strequal(current_token, "Ns") && material_open)
		{
            current_mtl->shiny = atof(strtok(NULL, " \t"));
		}
        // прозрачность
		else if( strequal(current_token, "d") && material_open)
		{
            current_mtl->trans = atof(strtok(NULL, " \t"));
		}
        // отражение
		else if( strequal(current_token, "r") && material_open)
		{
            current_mtl->reflect = atof(strtok(NULL, " \t"));
		}
		else if( strequal(current_token, "sharpness") && material_open)
		{
            current_mtl->glossy = atof(strtok(NULL, " \t"));
		}
		else if( strequal(current_token, "Ni") && material_open)
		{
			current_mtl->refract_index = atof( strtok(NULL, " \t"));
		}
		else if( strequal(current_token, "illum") && material_open)
		{
		}
        else if( strequal(current_token, "map_Kd") && material_open)
		{
            char ending[OBJ_FILENAME_LENGTH];
            char tex_filename[OBJ_FILENAME_LENGTH];

            memset(ending, '\0', sizeof(ending));
            memset(tex_filename, '\0', sizeof(tex_filename));
            memset(current_mtl->texture_filename, '\0', sizeof(current_mtl->texture_filename));

            strncpy(ending, strtok(NULL, " \t"), 200);
            strncpy(tex_filename, filename, strlen(filename) + 1);
            char *ptr_to_start = strrchr(tex_filename, '/') + 1;
            strncpy(ptr_to_start, ending, strlen(ending) + 1);

            //strncpy(current_mtl->texture_filename, strtok(NULL, " \t"), OBJ_FILENAME_LENGTH);
            strncpy(current_mtl->texture_filename, tex_filename, strlen(tex_filename) + 1);
		}
		else
		{
            /*fprintf(stderr, "Unknown command '%s' in material file %s at line %i:\n\t%s\n",
                    current_token, filename, line_number, current_line);*/
			//return 0;
		}
	}
	
	fclose(mtl_file_stream);

	return 1;

}

int obj_parse_obj_file(ObjGrowableSceneData *growable_data, const char *filename)
{
	FILE* obj_file_stream;
	int current_material = -1; 
	char *current_token = NULL;
	char current_line[OBJ_LINE_SIZE];
	int line_number = 0;
    // открытие сцены
    obj_file_stream = fopen(filename, "r");
	if(obj_file_stream == 0)
	{
        //fprintf(stderr, "Error reading file: %s\n", filename);
        OpeningFileException *ex = new OpeningFileException(QString(filename));
        ex->raise();
		return 0;
	}

    while (fgets(current_line, OBJ_LINE_SIZE, obj_file_stream) )
	{
		current_token = strtok( current_line, " \t\n\r");
		line_number++;
		
        // пропуск комментариев
		if( current_token == NULL || current_token[0] == '#')
			continue;

        else if(strequal(current_token, "v")) // vertex
		{
            //qDebug() << __LINE__;
            list_add_item(&growable_data->vertex_list, obj_parse_vector(), NULL);
		}
        else if(strequal(current_token, "vn")) //vertex normal
		{
            //qDebug() << __LINE__;
            list_add_item(&growable_data->vertex_normal_list, obj_parse_vector(), NULL);
		}
        else if(strequal(current_token, "vt")) //vertex texture
		{
            //qDebug() << __LINE__;
            list_add_item(&growable_data->vertex_texture_list, obj_parse_vector(), NULL);
		}
        else if(strequal(current_token, "f")) //face - полигон
		{
            static unsigned int a = 0;
            //qDebug() << __LINE__ << ++a;
            ObjFace *face = obj_parse_face(growable_data);
			face->material_index = current_material;
			list_add_item(&growable_data->face_list, face, NULL);
		}
        else if(strequal(current_token, "sp")) // sphere
		{
            ObjSphere *sphr = obj_parse_sphere(growable_data);
			sphr->material_index = current_material;
			list_add_item(&growable_data->sphere_list, sphr, NULL);
		}
        else if(strequal(current_token, "pl")) // plane
		{
            //qDebug() << __LINE__;
            ObjPlane *pl = obj_parse_plane(growable_data);
			pl->material_index = current_material;
			list_add_item(&growable_data->plane_list, pl, NULL);
		}
        else if(strequal(current_token, "p")) // point
		{
		}
        else if(strequal(current_token, "lp")) //light point source
		{
            ObjLightPoint *o = obj_parse_light_point(growable_data);
			o->material_index = current_material;
			list_add_item(&growable_data->light_point_list, o, NULL);
		}
		
        else if(strequal(current_token, "ld")) // light disc
		{
            ObjLightDisc *o = obj_parse_light_disc(growable_data);
			o->material_index = current_material;
			list_add_item(&growable_data->light_disc_list, o, NULL);
		}
		
        else if(strequal(current_token, "lq")) // light quad
		{
            ObjLightQuad *o = obj_parse_light_quad(growable_data);
			o->material_index = current_material;
			list_add_item(&growable_data->light_quad_list, o, NULL);
		}
		
        else if(strequal(current_token, "c")) // camera
		{
            growable_data->camera = (ObjCamera*) malloc(sizeof(ObjCamera));
			obj_parse_camera(growable_data, growable_data->camera);
		}
		
        else if(strequal(current_token, "usemtl")) // usemtl
		{
            //qDebug() << __LINE__;
			current_material = list_find(&growable_data->material_list, strtok(NULL, WHITESPACE));
		}
		
        else if(strequal(current_token, "mtllib")) // mtllib
		{
            //qDebug() << __LINE__;
            char ending[OBJ_FILENAME_LENGTH];
            char mtl_filename[OBJ_FILENAME_LENGTH];

            memset(ending, '\0', sizeof(ending));
            memset(mtl_filename, '\0', sizeof(mtl_filename));
            memset(growable_data->material_filename, '\0', sizeof(growable_data->material_filename));

            strncpy(ending, strtok(NULL, WHITESPACE), 200);
            strncpy(mtl_filename, filename, OBJ_FILENAME_LENGTH - 2);
            char *ptr_to_start = strrchr(mtl_filename, '/') + 1;
            strncpy(ptr_to_start, ending, strlen(ending) + 1);

            //strncpy(growable_data->material_filename, strtok(NULL, WHITESPACE), OBJ_FILENAME_LENGTH);
            strncpy(growable_data->material_filename, mtl_filename, strlen(mtl_filename) + 1);
			obj_parse_mtl_file(growable_data->material_filename, &growable_data->material_list);
			continue;
		}
		else if( strequal(current_token, "o") ) //object name
		{ }
		else if( strequal(current_token, "s") ) //smoothing
		{ }
		else if( strequal(current_token, "g") ) // group
		{ }		
		else
		{
            /* printf("Unknown command '%s' in scene code at line %i: \"%s\".\n",
                    current_token, line_number, current_line);
            char er_buf[300];
            int ok_mem = sprintf(er_buf, "Unknown command '%s' in scene code at line %i: \"%s\".\n",
                    current_token, line_number, current_line);
            if (ok_mem <= 0)
                throw std::string("");*/

            UnknownCommandException ex;
            ex.SetMessage("Unknown command " + QString(current_token) + " in scene code at line " +
                          QString::number(line_number) + ": " + current_line + ". ");
            ex.SetSourceFilename(__FILE__);
            ex.SetSourceLine(__LINE__);
            //ex.raise();
		}
	}

	fclose(obj_file_stream);
	
	return 1;
}


void obj_init_temp_storage(ObjGrowableSceneData *growable_data)
{
	list_make(&growable_data->vertex_list, 10, 1);
	list_make(&growable_data->vertex_normal_list, 10, 1);
	list_make(&growable_data->vertex_texture_list, 10, 1);
	
	list_make(&growable_data->face_list, 10, 1);
	list_make(&growable_data->sphere_list, 10, 1);
	list_make(&growable_data->plane_list, 10, 1);
	
	list_make(&growable_data->light_point_list, 10, 1);
	list_make(&growable_data->light_quad_list, 10, 1);
	list_make(&growable_data->light_disc_list, 10, 1);
	
	list_make(&growable_data->material_list, 10, 1);	
	
	growable_data->camera = NULL;
}

void obj_free_temp_storage(ObjGrowableSceneData *growable_data)
{
	obj_free_half_list(&growable_data->vertex_list);
	obj_free_half_list(&growable_data->vertex_normal_list);
	obj_free_half_list(&growable_data->vertex_texture_list);
	
	obj_free_half_list(&growable_data->face_list);
	obj_free_half_list(&growable_data->sphere_list);
	obj_free_half_list(&growable_data->plane_list);
	
	obj_free_half_list(&growable_data->light_point_list);
	obj_free_half_list(&growable_data->light_quad_list);
	obj_free_half_list(&growable_data->light_disc_list);
	
	obj_free_half_list(&growable_data->material_list);
}

void delete_obj_data(ObjSceneData *data_out)
{
	int i;
	
	for(i=0; i<data_out->vertex_count; i++)
		free(data_out->vertex_list[i]);
	free(data_out->vertex_list);
	for(i=0; i<data_out->vertex_normal_count; i++)
		free(data_out->vertex_normal_list[i]);
	free(data_out->vertex_normal_list);
	for(i=0; i<data_out->vertex_texture_count; i++)
		free(data_out->vertex_texture_list[i]);
	free(data_out->vertex_texture_list);

	for(i=0; i<data_out->face_count; i++)
		free(data_out->face_list[i]);
	free(data_out->face_list);
	for(i=0; i<data_out->sphere_count; i++)
		free(data_out->sphere_list[i]);
	free(data_out->sphere_list);
	for(i=0; i<data_out->plane_count; i++)
		free(data_out->plane_list[i]);
	free(data_out->plane_list);

	for(i=0; i<data_out->light_point_count; i++)
		free(data_out->light_point_list[i]);
	free(data_out->light_point_list);
	for(i=0; i<data_out->light_disc_count; i++)
		free(data_out->light_disc_list[i]);
	free(data_out->light_disc_list);
	for(i=0; i<data_out->light_quad_count; i++)
		free(data_out->light_quad_list[i]);
	free(data_out->light_quad_list);

	for(i=0; i<data_out->material_count; i++)
		free(data_out->material_list[i]);
	free(data_out->material_list);

	free(data_out->camera);
}

void obj_copy_to_out_storage(ObjSceneData *data_out, ObjGrowableSceneData *growable_data)
{
	data_out->vertex_count = growable_data->vertex_list.item_count;
	data_out->vertex_normal_count = growable_data->vertex_normal_list.item_count;
	data_out->vertex_texture_count = growable_data->vertex_texture_list.item_count;

	data_out->face_count = growable_data->face_list.item_count;
	data_out->sphere_count = growable_data->sphere_list.item_count;
	data_out->plane_count = growable_data->plane_list.item_count;

	data_out->light_point_count = growable_data->light_point_list.item_count;
	data_out->light_disc_count = growable_data->light_disc_list.item_count;
	data_out->light_quad_count = growable_data->light_quad_list.item_count;

	data_out->material_count = growable_data->material_list.item_count;
	
    data_out->vertex_list = (ObjVector**)growable_data->vertex_list.items;
    data_out->vertex_normal_list = (ObjVector**)growable_data->vertex_normal_list.items;
    data_out->vertex_texture_list = (ObjVector**)growable_data->vertex_texture_list.items;

    data_out->face_list = (ObjFace**)growable_data->face_list.items;
    data_out->sphere_list = (ObjSphere**)growable_data->sphere_list.items;
    data_out->plane_list = (ObjPlane**)growable_data->plane_list.items;

    data_out->light_point_list = (ObjLightPoint**)growable_data->light_point_list.items;
    data_out->light_disc_list = (ObjLightDisc**)growable_data->light_disc_list.items;
    data_out->light_quad_list = (ObjLightQuad**)growable_data->light_quad_list.items;
	
    data_out->material_list = (ObjMaterial**)growable_data->material_list.items;
	
	data_out->camera = growable_data->camera;
}

int parse_obj_scene(ObjSceneData *data_out, const char *filename)
{
    ObjGrowableSceneData growable_data;

	obj_init_temp_storage(&growable_data);
	if( obj_parse_obj_file(&growable_data, filename) == 0)
		return 0;

	obj_copy_to_out_storage(data_out, &growable_data);
	obj_free_temp_storage(&growable_data);
	return 1;
}

