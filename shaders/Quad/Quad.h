#pragma once
#include "Helpers/ShaderHelper.h"
#include "Helpers/GlError.h"
#include "Helpers/lodepng.h"


/// A unit quad (a square from -0.5 to 0.5) with a texture on it.
/// The pumpkin and the bats are both quads; only the texture differs.
class Quad {
private:
	GLuint _vao; ///< vertex array object
	GLuint _pid; ///< GLSL shader program ID 
	GLuint _vbo_vpoint; ///< memory buffer
	GLuint _vbo_vtexcoord; ///< memory buffer
	GLuint _tex; ///< Texture ID
	ShaderHelper shader;
public:
	void init(const std::string texture_file) {

		///--- Compile the shaders
		_pid = shader.load_shaders("Quad/vshader.glsl", "Quad/fshader.glsl");
		if (!_pid) exit(EXIT_FAILURE);
		glUseProgram(_pid);

		///--- Vertex one vertex Array
		glGenVertexArrays(1, &_vao);
		glBindVertexArray(_vao);

		///--- Vertex coordinates
		{
			const GLfloat vpoint[] = { /*V1*/ -0.5f, -0.5f, 0.0f,
				/*V2*/ +0.5f, -0.5f, 0.0f,
				/*V3*/ -0.5f, +0.5f, 0.0f,
				/*V4*/ +0.5f, +0.5f, 0.0f };
			///--- Buffer
			glGenBuffers(1, &_vbo_vpoint);
			glBindBuffer(GL_ARRAY_BUFFER, _vbo_vpoint);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vpoint), vpoint, GL_STATIC_DRAW);

			///--- Attribute
			GLuint vpoint_id = glGetAttribLocation(_pid, "vpoint");
			glEnableVertexAttribArray(vpoint_id);
			glVertexAttribPointer(vpoint_id, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		}

		///--- Texture coordinates
		{
			const GLfloat vtexcoord[] = { /*V1*/ 0.0f, 0.0f,
				/*V2*/ 1.0f, 0.0f,
				/*V3*/ 0.0f, 1.0f,
				/*V4*/ 1.0f, 1.0f };

			///TODO1: Allocate GPU memory for the texture coordinates.
			// Copy the three "Buffer" lines from the vertex coordinates block above, and change:
			//   _vbo_vpoint -> _vbo_vtexcoord   (our second buffer)
			//   vpoint      -> vtexcoord        (the array just above)
			// What each call does:
			//   glGenBuffers(1, &...)                       get a new buffer name
			//   glBindBuffer(GL_ARRAY_BUFFER, ...)          make it the current vertex buffer
			//   glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW)
			//                                               copy the array into it (size in bytes: use sizeof)

			///TODO2: Create the vertex attribute "vtexcoord" (the "in vec2 vtexcoord;" in Quad/vshader.glsl).
			// Copy the three "Attribute" lines from the vertex coordinates block above, and change:
			//   the variable name: vpoint_id -> vtexcoord_id
			//   the string "vpoint" -> "vtexcoord"  (must match the shader name exactly)
			//   the 2nd argument of glVertexAttribPointer: how many floats per vertex?
			//     A position is (x, y, z) = 3. A texture coordinate is (u, v) = ?
			// Keep stride 0 and offset (void*)0: this buffer holds only texture coordinates.
			// Important: TODO1 must come first. glVertexAttribPointer reads from the buffer
			// that is bound to GL_ARRAY_BUFFER at that moment.

		}

		///--- Load texture
		this->loadTextures(texture_file);

		///--- to avoid the current object being polluted
		glBindVertexArray(0);
		glUseProgram(0);

	}

	void cleanup() {

		glBindVertexArray(0);
		glUseProgram(0);
		glDeleteBuffers(1, &_vbo_vpoint);
		glDeleteBuffers(1, &_vbo_vtexcoord);
		glDeleteProgram(_pid);
		glDeleteVertexArrays(1, &_vao);
		glDeleteTextures(1, &_tex);

	}

	/// id tells the shaders which object is drawn: 0 is the pumpkin, 1, 2, 3 are the bats
	void draw(int id) {

		glUseProgram(_pid);
		glBindVertexArray(_vao);
		///--- Uniforms for animation
		float t = glfwGetTime();
		glUniform1f(glGetUniformLocation(_pid, "time"), t);
		glUniform1i(glGetUniformLocation(_pid, "id"), id);

		///--- Bind textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _tex);

		///--- Draw
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
		glUseProgram(0);

	}

	void loadTextures(const std::string filename) {
		// Used snippet from https://raw.githubusercontent.com/lvandeve/lodepng/master/examples/example_decode.cpp
		std::vector<unsigned char> image; //the raw pixels
		unsigned width, height;
		//decode
		unsigned error = lodepng::decode(image, width, height, filename);
		//if there's an error, display it
		if (error) {
			std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << " (" << filename << ")" << std::endl;
			exit(EXIT_FAILURE);
		}
		//the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...

		// unfortunately they are upside down...lets fix that
		unsigned char* row = new unsigned char[4 * width];
		for (int i = 0; i < int(height) / 2; ++i) {
			memcpy(row, &image[4 * i * width], 4 * width * sizeof(unsigned char));
			memcpy(&image[4 * i * width], &image[image.size() - 4 * (i + 1) * width], 4 * width * sizeof(unsigned char));
			memcpy(&image[image.size() - 4 * (i + 1) * width], row, 4 * width * sizeof(unsigned char));
		}
		delete[] row;

		glBindVertexArray(_vao);
		check_error_gl();

		glUseProgram(_pid);

		GLuint tex_id = glGetUniformLocation(_pid, "tex");
		check_error_gl();
		glUniform1i(tex_id, 0);
		check_error_gl();

		glGenTextures(1, &_tex);
		glBindTexture(GL_TEXTURE_2D, _tex);

		check_error_gl();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		check_error_gl();

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
			width, height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
		check_error_gl();

	}
};
