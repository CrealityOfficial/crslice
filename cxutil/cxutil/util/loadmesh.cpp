#include "loadmesh.h"
#include "cxutil/slicer/mesh.h"
#include "cxutil/slicer/MeshGroup.h"
#include "cxutil/util/logoutput.h"
#include "cxutil/util/string.h"

namespace cxutil
{
	bool loadMeshIntoMeshGroup(MeshGroup* meshgroup, const char* filename,
		const FMatrix4x4& transformation, Settings* object_parent_settings)
	{
        const char* ext = strrchr(filename, '.');
        if (ext && (strcmp(ext, ".stl") == 0 || strcmp(ext, ".STL") == 0))
        {
            Mesh* mesh = loadMeshSTL(filename, transformation, object_parent_settings);
            if (mesh)
            {
                meshgroup->meshes.push_back(mesh);
                return true;
            }
        }
        return false;
	}

    /* Custom fgets function to support Mac line-ends in Ascii STL files. OpenSCAD produces this when used on Mac */
    void* fgets_(char* ptr, size_t len, FILE* f)
    {
        while (len && fread(ptr, 1, 1, f) > 0)
        {
            if (*ptr == '\n' || *ptr == '\r')
            {
                *ptr = '\0';
                return ptr;
            }
            ptr++;
            len--;
        }
        return nullptr;
    }

    bool loadMeshSTL_ascii(Mesh* mesh, const char* filename, const FMatrix4x4& matrix)
    {
        FILE* f = fopen(filename, "rt");
        char buffer[1024];
        FPoint3 vertex;
        int n = 0;
        Point3 v0(0, 0, 0), v1(0, 0, 0), v2(0, 0, 0);
        while (fgets_(buffer, sizeof(buffer), f))
        {
            if (sscanf(buffer, " vertex %f %f %f", &vertex.x, &vertex.y, &vertex.z) == 3)
            {
                n++;
                switch (n)
                {
                case 1:
                    v0 = matrix.apply(vertex);
                    break;
                case 2:
                    v1 = matrix.apply(vertex);
                    break;
                case 3:
                    v2 = matrix.apply(vertex);
                    mesh->addFace(v0, v1, v2);
                    n = 0;
                    break;
                }
            }
        }
        fclose(f);
        mesh->finish();
        return true;
    }

    bool loadMeshSTL_binary(Mesh* mesh, const char* filename, const FMatrix4x4& matrix)
    {
        FILE* f = fopen(filename, "rb");

        fseek(f, 0L, SEEK_END);
        long long file_size = ftell(f); //The file size is the position of the cursor after seeking to the end.
        rewind(f); //Seek back to start.
        size_t face_count = (file_size - 80 - sizeof(uint32_t)) / 50; //Subtract the size of the header. Every face uses exactly 50 bytes.

        char buffer[80];
        //Skip the header
        if (fread(buffer, 80, 1, f) != 1)
        {
            fclose(f);
            return false;
        }

        uint32_t reported_face_count;
        //Read the face count. We'll use it as a sort of redundancy code to check for file corruption.
        if (fread(&reported_face_count, sizeof(uint32_t), 1, f) != 1)
        {
            fclose(f);
            return false;
        }
        if (reported_face_count != face_count)
        {
            LOGW("Face count reported by file (%s) is not equal to actual face count (%s). File could be corrupt!\n", std::to_string(reported_face_count).c_str(), std::to_string(face_count).c_str());
        }

        //For each face read:
        //float(x,y,z) = normal, float(X,Y,Z)*3 = vertexes, uint16_t = flags
        // Every Face is 50 Bytes: Normal(3*float), Vertices(9*float), 2 Bytes Spacer
        mesh->faces.reserve(face_count);
        mesh->vertices.reserve(face_count);
        for (unsigned int i = 0; i < face_count; i++)
        {
            if (fread(buffer, 50, 1, f) != 1)
            {
                fclose(f);
                return false;
            }
            float* v = ((float*)buffer) + 3;

            //if (i == 566)
            //    printf("");
            Point3 v0 = matrix.apply(FPoint3(v[0], v[1], v[2]));
            Point3 v1 = matrix.apply(FPoint3(v[3], v[4], v[5]));
            Point3 v2 = matrix.apply(FPoint3(v[6], v[7], v[8]));
            mesh->addFace(v0, v1, v2);
        }
        fclose(f);
        mesh->finish();
        return true;
    }

    Mesh* loadMeshSTL(const char* filename, const FMatrix4x4& matrix, Settings* object_parent_settings)
    {
        FILE* f = fopen(filename, "r");
        if (f == nullptr)
        {
            return nullptr;
        }

        //assign filename to mesh_name
        Mesh* mesh = new Mesh(object_parent_settings);
        mesh->mesh_name = filename;

        //Skip any whitespace at the beginning of the file.
        unsigned long long num_whitespace = 0; //Number of whitespace characters.
        unsigned char whitespace;
        if (fread(&whitespace, 1, 1, f) != 1)
        {
            fclose(f);
            return nullptr;
        }
        while (isspace(whitespace))
        {
            num_whitespace++;
            if (fread(&whitespace, 1, 1, f) != 1)
            {
                fclose(f);
                return nullptr;
            }
        }
        fseek(f, num_whitespace, SEEK_SET); //Seek to the place after all whitespace (we may have just read too far).

        char buffer[6];
        if (fread(buffer, 5, 1, f) != 1)
        {
            fclose(f);
            return nullptr;
        }
        fclose(f);

        buffer[5] = '\0';

        bool load_success = false;
        if (cxutil::stringcasecompare(buffer, "solid") == 0)
        {
            load_success = loadMeshSTL_ascii(mesh, filename, matrix);
            if (load_success)
            {
                // This logic is used to handle the case where the file starts with
                // "solid" but is a binary file.
                if (mesh->faces.size() < 1)
                {
                    mesh->clear();
                    load_success = loadMeshSTL_binary(mesh, filename, matrix);
                }
            }
        }
        load_success = loadMeshSTL_binary(mesh, filename, matrix);

        if (load_success)
            return mesh;

        delete mesh;
        return nullptr;
    }
}