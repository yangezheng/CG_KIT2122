#include <cglib/core/glheaders.h>
#include <cglib/gl/util.h>

void
create_cube(unsigned *vao, unsigned *buf_cube_vert, unsigned *buf_cube_idx)
{
	static float cube_vertices[][3] = {
		{ -1, -1, -1 },
		{  1, -1, -1 },
		{  1,  1, -1 },
		{ -1,  1, -1 },

		{ -1, -1,  1 },
		{  1, -1,  1 },
		{  1,  1,  1 },
		{ -1,  1,  1 },
	};

	static unsigned int cube_indices[] = {
		0, 1, 2, 2, 3, 0,
		1, 5, 6, 6, 2, 1,
		4, 5, 6, 6, 7, 4,
		0, 3, 7, 7, 4, 0,
		2, 3, 7, 2, 7, 6,
		0, 4, 5, 5, 1, 0
	};

	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);

	glGenBuffers(1, buf_cube_vert);
	glGenBuffers(1, buf_cube_idx);

	glBindBuffer(GL_ARRAY_BUFFER, *buf_cube_vert);
	glBufferData(GL_ARRAY_BUFFER, sizeof cube_vertices, cube_vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *buf_cube_idx);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof cube_indices, cube_indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);
}

void
create_cube_lines(unsigned *vao, unsigned *buf_cube_vert, unsigned *buf_cube_idx)
{
	static float cube_vertices[][3] = {
		{ -1, -1, -1 },
		{  1, -1, -1 },
		{  1,  1, -1 },
		{ -1,  1, -1 },

		{ -1, -1,  1 },
		{  1, -1,  1 },
		{  1,  1,  1 },
		{ -1,  1,  1 },
	};

	static unsigned int cube_indices[] = {
		0, 1, 1, 2, 2, 3, 3, 0,

		4, 5, 5, 6, 6, 7, 7, 4,

		0, 4, 1, 5, 2, 6, 3, 7
	};

	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);

	glGenBuffers(1, buf_cube_vert);
	glGenBuffers(1, buf_cube_idx);

	glBindBuffer(GL_ARRAY_BUFFER, *buf_cube_vert);
	glBufferData(GL_ARRAY_BUFFER, sizeof cube_vertices, cube_vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *buf_cube_idx);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof cube_indices, cube_indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);
}
