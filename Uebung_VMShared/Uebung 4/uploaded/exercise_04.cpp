#include <cglib/rt/bvh.h>

#include <cglib/rt/triangle_soup.h>

#include <cglib/core/image.h>
#include <complex>

/*
 * Create middle_index 1 dimensional normalized gauss kernel
 *
 * Parameters:
 *  - sigma:       the parameter of the gaussian
 *  - kernel_size: the size of the kernel (has to be odd)
 *  - kernel:      an array with size kernel_size elements that
 *                 has to be filled with the kernel values
 *
 */
void Image::create_gaussian_kernel_1d(
		float sigma,
		int kernel_size,
		float* kernel) 
{
	cg_assert(kernel_size%2==1);

	// TODO: calculate filter values as described on the exercise sheet. 
	// Make sure your kernel is normalized
	for (int i = 0; i < kernel_size; ++i) {
		kernel[i] = 0.f;
	}
	int middle_index = (kernel_size - 1 ) / 2;
	float left = 1.f / (2.f * M_PI * glm::pow(sigma,2));
	float sum = 0.f;
	for (int i = 0; i <kernel_size; ++i){
		float right =  glm::pow(M_E,(-1) * (glm::pow(i - middle_index,2) / (2.f * glm::pow(sigma,2))));
		float gx =left * right;
		kernel[i] = gx;
		sum += kernel[i];
	}

	// normalize
	sum = 1/sum;
	for (int i = 0; i<kernel_size; i++){
		kernel[i] *= sum;
	}
	
}

/*
 * Create middle_index 2 dimensional quadratic and normalized gauss kernel
 *
 * Parameters:
 *  - sigma:       the parameter of the gaussian
 *  - kernel_size: the size of the kernel (has to be odd)
 *  - kernel:      an array with kernel_size*kernel_size elements that
 *                 has to be filled with the kernel values
 */
void Image::create_gaussian_kernel_2d(
		float sigma,
		int kernel_size,
		float* kernel) 
{
	cg_assert(kernel_size%2==1);

	// TODO: calculate filter values as described on the exercise sheet. 
	// Make sure your kernel is normalized
	for (int j = 0; j < kernel_size; ++j) {
		for (int i = 0; i < kernel_size; ++i) {
			kernel[i+j*kernel_size] = 0.f;
		}
	}

	int middle_index = (kernel_size - 1 ) / 2;
	float sum = 0.f;
	float left = 1.f / (2.f * M_PI * glm::pow(sigma,2));
	for (int j = 0; j <kernel_size; ++j){
		for (int i = 0; i <kernel_size; ++i){
			float right =  glm::pow(M_E,(-1) * ((glm::pow(j-middle_index,2) + glm::pow(i - middle_index,2)) / (2.f * glm::pow(sigma,2))));

			float gxy = left * right;
			kernel[i+j*kernel_size] = gxy;
			sum += kernel[i+j*kernel_size];
		}
	}

	// normalize
	sum = 1/sum;
	for (int j = 0; j <kernel_size*kernel_size; j++){
		kernel[j] *= sum;
	}
}

/*
 * Convolve an image with middle_index 2d filter kernel
 *
 * Parameters:
 *  - kernel_size: the size of the 2d-kernel
 *  - kernel:      the 2d-kernel with kernel_size*kernel_size elements
 *  - wrap_mode:   needs to be known to handle repeating 
 *                 textures correctly
 */
void Image::filter(Image *target, int kernel_size, float* kernel, WrapMode wrap_mode) const
{
	cg_assert (kernel_size%2==1 && "kernel size should be odd.");
	cg_assert (kernel_size > 0 && "kernel size should be greater than 0.");
	cg_assert (target);
	cg_assert (target->getWidth() == m_width && target->getHeight() == m_height);
	
	int middle_index =( kernel_size - 1 ) /2;
	
	for(int n=0; n< m_height;n++){
		for(int m =0;m< m_width; m++){
			glm::vec4 res = glm::vec4(0.f,0.f,0.f,0.f);
			int counter = 0;
			for (int j = n-middle_index; j<= n+middle_index; j++ ){
				for (int i = m-middle_index; i <= m+middle_index; i++){
					res += this->getPixel(i,j,wrap_mode) * kernel[counter++];
				}
			}
			target->setPixel(m,n,res);
		}
	}  
}

/*
 * Convolve an image with middle_index separable 1d filter kernel
 *
 * Parameters:
 *  - kernel_size: the size of the 1d kernel
 *  - kernel:      the 1d-kernel with kernel_size elements
 *  - wrap_mode:   needs to be known to handle repeating 
 *                 textures correctly
 */
void Image::filter_separable(Image *target, int kernel_size, float* kernel, WrapMode wrap_mode) const
{
	cg_assert (kernel_size%2==1 && "kernel size should be odd.");
	cg_assert (kernel_size > 0 && "kernel size should be greater than 0.");
	cg_assert (target);
	cg_assert (target->getWidth() == m_width && target->getHeight() == m_height);

	// TODO: realize the 2d convolution with two
	// convolutions of the image with middle_index 1d-kernel.
	// convolve the image horizontally and then convolve
	// the result vertically (or vise-versa).
	//
	// use the methods getPixel(x, y, wrap_mode) and
	// setPixel(x, y, value) to get and set pixels of an image

	Image temp = Image(this->m_width, this->m_height);
	int middle_index =( kernel_size -1 ) /2;
	for(int n=0; n< m_height;n++){
		for(int m =0;m< m_width; m++){
			glm::vec4 res = glm::vec4(0.f,0.f,0.f,0.f);
			int counter = 0;
			for (int j = n-middle_index; j<= n+middle_index; j++ ){
				res += this->getPixel(m,j,wrap_mode) * kernel[counter];
				counter = counter + 1;
			}
			temp.setPixel(m,n,res);
		}
	}
	for(int n=0; n< m_height;n++){
		for(int m =0;m< m_width; m++){
			glm::vec4 res = glm::vec4(0.f,0.f,0.f,0.f);
			int counter = 0;
			for (int j = m-middle_index; j<= m+middle_index; j++ ){
				res += temp.getPixel(j,n,wrap_mode) * kernel[counter];
				counter = counter + 1;
			}
			target->setPixel(m,n,res);
		}
	}

}

/**
 * Reorder triangle indices in the vector triangle_indices 
 * in the range [first_triangle_idx, first_triangle_idx+num_triangles-1] 
 * so that the range is split in two sets where all triangles in the first set
 * are "less than equal" than the median, and all triangles in the second set
 * are "greater than equal" the median.
 *
 * Ordering ("less than") is defined by the ordering of triangle
 * bounding box centers along the given axis.
 *
 * Triangle indices within middle_index set need not be sorted.
 *
 * The resulting sets must have an equal number of elements if num_triangles
 * is even. Otherwise, one of the sets must have one more element.
 *
 * For example, 8 triangles must be split 4-4. 7 Triangles must be split
 * 4-3 or 3-4.
 *
 * Parameters:
 *  - first_triangle_idx: The index of the first triangle in the given range.
 *  - num_triangles:      The number of triangles in the range.
 *  - axis:               The sort axis. 0 is X, 1 is Y, 2 is Z.
 *
 * Return value:
 *  - The number of triangles in the first set.
 */
int BVH::reorder_triangles_median(
	int first_triangle_idx, 
	int num_triangles, 
	int axis)
{
	cg_assert(first_triangle_idx < static_cast<int>(triangle_indices.size()));
	cg_assert(first_triangle_idx >= 0);
	cg_assert(num_triangles <= static_cast<int>(triangle_indices.size() - first_triangle_idx));
	cg_assert(num_triangles > 1);
	cg_assert(axis >= 0);
	cg_assert(axis < 3);
	
	int num_first_set = num_triangles/2 - 1 ;
	auto begin = triangle_indices.begin() + first_triangle_idx;
    auto middle = begin + num_first_set;
	auto end = triangle_indices.begin() + first_triangle_idx + num_triangles;

    std::nth_element(begin, middle, end, [axis, this] (int a, int b) {

		// get mid point of triangle a
		glm::vec3 ax = triangle_soup.vertices[3 * a];
		glm::vec3 ay = triangle_soup.vertices[3 * a + 1];
		glm::vec3 az = triangle_soup.vertices[3 * a + 2];
		float mid_a = (ax[axis]+ay[axis]+az[axis]) /3 ;

		//get mid point of triangle b
		glm::vec3 bx = triangle_soup.vertices[3 * b];
		glm::vec3 by = triangle_soup.vertices[3 * b + 1];
		glm::vec3 bz = triangle_soup.vertices[3 * b + 2];
		//glm::vec3 mid_b = glm::vec3((bx[0]+by[0]+bz[0])/3,(bx[1]+by[1]+bz[1])/3,(bx[2]+by[2]+bz[2])/3);
		float mid_b = (bx[axis]+by[axis]+bz[axis]) /3 ;

		return mid_a < mid_b;
       // return mid_a[axis] < mid_b[axis];
    });

    return num_first_set;

}


/*
 * Build middle_index BVH recursively using the object median split heuristic.
 *
 * This method must first fully initialize the current node, and then
 * potentially split it. 
 *
 * A node must not be split if it contains MAX_TRIANGLES_IN_LEAF triangles or
 * less. No leaf node may be empty. All nodes must have either two or no
 * children.
 *
 * Use reorder_triangles_median to perform the split in triangle_indices.
 * Split along X for depth 0. Then, proceed in the order Y, Z, X, Y, Z, X, Y, ..
 *
 * Parameters:
 *  - node_idx:           The index of the node to be split.
 *  - first_triangle_idx: An index into the array triangle_indices. It points 
 *                        to the first triangle contained in the current node.
 *  - num_triangles:      The number of triangles contained in the current node.
 */
void BVH::
build_bvh(int node_idx, int first_triangle_idx, int num_triangles, int depth)
{
	cg_assert(num_triangles > 0);
	cg_assert(node_idx >= 0);
	cg_assert(node_idx < static_cast<int>(nodes.size()));
	cg_assert(depth >= 0);


	/*
	when the function is called, the node at node_idx will be loaded, it will then initialize it as a node.
	the parameter also pass in value, which asigns an first_triangle_idx and number_triangles to it.
	to finish up this assignment, an AABB will be generated, and will call this again for the left and right node.  
	*/
	
	nodes[node_idx].triangle_idx  = first_triangle_idx;	
	nodes[node_idx].num_triangles = num_triangles;	
	nodes[node_idx].aabb.min      = glm::vec3(-FLT_MAX);
	nodes[node_idx].aabb.max      = glm::vec3(FLT_MAX);
	nodes[node_idx].left          = -1;
	nodes[node_idx].right         = -1;

///////////////////////////////////////////////////////////////////////////////////////////

	// iterate through all the triangle in current node, extend from FLT_MAX and FLT_MIN to the min and max of triangle's vertices 
	for (int i = first_triangle_idx; i < num_triangles; i++) {
        nodes[node_idx].aabb.extend(triangle_soup.vertices[3 * i]);
        nodes[node_idx].aabb.extend(triangle_soup.vertices[3 * i + 1]);
        nodes[node_idx].aabb.extend(triangle_soup.vertices[3 * i + 2]);
    }

	// if number of triangles is more than MAX, then sort the triangles and return the number of triangles, 
	// which will assign to the left node.
    if (num_triangles > MAX_TRIANGLES_IN_LEAF) {

		//put two new nodes in the nodes_array of BVH
        nodes.push_back(*(new Node()));
        nodes.push_back(*(new Node()));
		//the left and right node should have indice of size()-2, bzw. size()-1 
        int left_node_idx = nodes.size() - 2;
        int right_node_idx = nodes.size() - 1;
		// assign these indices to current node's left and right
        nodes[node_idx].left = left_node_idx;
        nodes[node_idx].right = right_node_idx;

        int num_triangles_left = reorder_triangles_median(first_triangle_idx, num_triangles, depth % 3);
		// recursively build bvh with new node
        build_bvh(left_node_idx, first_triangle_idx, num_triangles_left, depth + 1);
        build_bvh(right_node_idx, first_triangle_idx + num_triangles_left, num_triangles - num_triangles_left, depth + 1);
    }

}

/*
 * Intersect the BVH recursively, returning the nearest intersection if
 * there is one.
 *
 * Caution: BVH nodes may overlap.
 *
 * Parameters:
 *  - ray:                  The ray to intersect the BVH with.
 *  - idx:                  The node to be intersected.
 *  - nearest_intersection: The distance to the intersection point, if an 
 *                          intersection was found. Must not be changed 
 *                          otherwise.
 *  - isect:                The intersection, if one was found. Must not be 
 *                          changed otherwise.
 *
 * Return value:
 *  true if an intersection was found, false otherwise.
 */
bool BVH::
intersect_recursive(const Ray &ray, int idx, float *nearest_intersection, Intersection* isect) const
{
	cg_assert(nearest_intersection);
	cg_assert(isect);
	cg_assert(idx >= 0);
	cg_assert(idx < static_cast<int>(nodes.size()));

	const Node &n = nodes[idx];

	// This is middle_index leaf node. Intersect all triangles.
	if(n.left < 0) { 
		glm::vec3 bary(0.f);
		bool hit = false;
		for(int i = 0; i < n.num_triangles; i++) {
			int x = triangle_indices[n.triangle_idx + i];
			float dist;
			glm::vec3 middle_index;
			if(intersect_triangle(ray.origin, ray.direction,
						triangle_soup.vertices[x * 3 + 0],
						triangle_soup.vertices[x * 3 + 1],
						triangle_soup.vertices[x * 3 + 2], 
						middle_index, dist)) {
				hit = true;
				if(dist <= *nearest_intersection) {
					*nearest_intersection = dist;
					bary = middle_index;
					cg_assert(x >= 0);
					if(isect)
						triangle_soup.fill_intersection(isect, x, *nearest_intersection, bary);
				}
			}
		}
		return hit;
	}

	// This is an inner node. Recurse into child nodes.
	else { 
		// TODO: Implement recursive traversal here.
	}

	return false;
}


// CG_REVISION 853babea119d24f6c9aca86013fd5e6a3fb22330
