#include <vm_contract.h>
#include <utility.h>
#include <heap_manager.h>

extern heap_t* kheap;

// private functions

// interval tree functions

static uint32_t get_node_height(vm_area_node_t* node)
{
	if (!node)
		return 0;
	return node->height;
}

static uint32_t get_node_max(vm_area_node_t* node)
{
	if (!node)
		return 0;
	return node->max;
}

static int get_node_balance(vm_area_node_t* node)
{
	if (!node)
		return 0;
	return get_node_height(node->left) - get_node_height(node->right);
}

static vm_area_node_t* avl_rotate_right(vm_area_node_t* n)
{
	vm_area_node_t* x = n->left;
	vm_area_node_t* T2 = x->right;

	// rotate
	x->right = n;
	n->left = T2;

	n->height = max(get_node_height(n->left), get_node_height(n->right)) + 1;
	n->max = max(n->area.end_addr, max(get_node_max(n->left), get_node_max(n->right)));

	x->height = max(get_node_height(x->left), get_node_height(x->right)) + 1;
	x->max = max(x->area.end_addr, max(get_node_max(x->left), get_node_max(x->right)));

	return x;
}

static vm_area_node_t* avl_rotate_left(vm_area_node_t* n)
{
	vm_area_node_t* y = n->right;
	vm_area_node_t* T2 = y->left;

	// rotate
	y->left = n;
	n->right = T2;

	n->height = max(get_node_height(n->left), get_node_height(n->right)) + 1;
	n->max = max(n->area.end_addr, max(get_node_max(n->left), get_node_max(n->right)));

	y->height = max(get_node_height(y->left), get_node_height(y->right)) + 1;
	y->max = max(y->area.end_addr, max(get_node_max(y->left), get_node_max(y->right)));

	return y;
}

static vm_area_node_t* avl_insert(vm_area_node_t* tree, vm_area_node_t* ins)
{
	if (!tree)
		return ins;
	
	uint32_t key = ins->area.start_addr;

	if (key < tree->area.start_addr)
		tree->left = avl_insert(tree->left, ins);
	else if (key > tree->area.start_addr)
		tree->right = avl_insert(tree->right, ins);
	else
		return tree;		// duplicate

	tree->height = 1 + max(get_node_height(tree->left), get_node_height(tree->right));
	tree->max = max(tree->area.end_addr, max(get_node_max(tree->left), get_node_max(tree->right)));

	int balance = get_node_balance(tree);

	if (balance > 1 && key < tree->left->area.start_addr)
		return avl_rotate_right(tree);

	if (balance < -1 && key > tree->right->area.start_addr)
		return avl_rotate_left(tree);

	if (balance > 1 && key > tree->left->area.start_addr)
	{
		tree->left = avl_rotate_left(tree->left);
		return avl_rotate_right(tree);
	}

	if (balance < -1 && key < tree->right->area.start_addr)
	{
		tree->right = avl_rotate_right(tree->right);
		return avl_rotate_left(tree);
	}

	return tree;
}

static vm_area_node_t* interval_intersect(vm_area_node_t* root, vm_area_t* area)
{
	if (root == NULL)
		return 0;

	// check intersection with the root interval
	if (vm_area_intersect(&root->area, area))
		return root;

	// if the left child has the capacity to overlap (due to max), try it
	if (root->left && root->left->max >= area->start_addr)
		return interval_intersect(root->left, area);

	return interval_intersect(root->right, area);
}

static vm_area_node_t* vm_area_node_create(vm_area_t area)
{
	vm_area_node_t* new_node = (vm_area_node_t*)heap_alloc(kheap, sizeof(vm_area_node_t));
	if(new_node == 0)
		return 0;

	new_node->area = area;
	new_node->height = 1;
	new_node->max = area.end_addr;
	new_node->left = new_node->right = 0;

	return new_node;
}

static void vm_contract_print_rec(vm_area_node_t* root)
{
	if(root == 0)
		return;

	vm_contract_print_rec(root->left);
	vm_area_print(&root->area);
	vm_contract_print_rec(root->right);
}

// public functions

void vm_contract_init(vm_contract_t* c)
{
	c->root = 0;
}

error_t vm_contract_add_area(vm_contract_t* c, vm_area_t new_area)
{
	// sanity check
	if (!vm_area_is_ok(&new_area))
	{
		// set_last_error(EINVAL, VM_CONTRACT_BAD_ARGUMENTS, EO_VM_CONTRACT);
		return ERROR_OCCUR;
	}

	if(interval_intersect(c->root, &new_area))
	{
		// set error. area intersects with existing one
		return ERROR_OCCUR;
	}

	vm_area_node_t* new_node = vm_area_node_create(new_area);

	if(new_node == 0)
	{
		// memory could not be allocated
		return ERROR_OCCUR;
	}

	c->root = avl_insert(c->root, new_node);
	return ERROR_OK;
}

error_t vm_contract_remove_area(vm_contract_t* c, vm_area_t* area)
{
	// TODO: Add remove code


	// if (vm_area_is_removable(area) == false)
	// {
	// 	DEBUG("Attempting to remove non-removable vm_area_t");
	// 	set_last_error(EINVAL, VM_CONTRACT_AREA_NON_REMOVABLE, EO_VM_CONTRACT);
	// 	return ERROR_OCCUR;
	// }

	// if (ordered_vector_remove(&c->contract, ordered_vector_find(&c->contract, *area)) == false)
	// {
	// 	set_last_error(EINVAL, VM_CONTRACT_NOT_FOUND, EO_VM_CONTRACT);
	// 	return ERROR_OCCUR;
	// }

	return ERROR_OK;
}

error_t vm_contract_expand_area(vm_contract_t* c, vm_area_t* area, uint32_t length)
{
	// TODO: fix this function
	// Just remove and re-insert the entry

	// if (area == 0 || c == 0 || vm_area_is_removable(area) == false)
	// {
	// 	set_last_error(EINVAL, VM_CONTRACT_BAD_ARGUMENTS, EO_VM_CONTRACT);
	// 	return ERROR_OCCUR;
	// }

	// if (length % vmmngr_get_page_size() != 0)
	// {
	// 	// TODO: Remove this
	// 	PANIC("Area expansion failed due to unaligned length");		// in release mode can be ommited
	// 	set_last_error(EINVAL, VM_CONTRACT_BAD_ARGUMENTS, EO_VM_CONTRACT);
	// 	return ERROR_OCCUR;
	// }

	// uint32 index = ordered_vector_find(&c->contract, *area);
	// if (index >= c->contract.count)		// area was not found
	// {
	// 	set_last_error(EINVAL, VM_CONTRACT_NOT_FOUND, EO_VM_CONTRACT);
	// 	return ERROR_OCCUR;
	// }

	// // no need to test index - 1 and index + 1 validity as both 0 and count - 1 areas are non-removable. See above if
	// vm_area_t* adjacent;
	// vm_area_t temp = *area;

	// if (vm_area_grows_down(area))
	// {
	// 	adjacent = &c->contract.data[index - 1];
	// 	temp.start_addr -= length;
	// }
	// else
	// {
	// 	adjacent = &c->contract.data[index + 1];
	// 	temp.end_addr += length;
	// }

	// // sanity check before altering stucture data
	// /*if (vm_area_is_ok(&temp) == false)
	// {
	// 	return false;
	// }*/

	// if (vm_area_intersects(adjacent, &temp))		// on intersection leave the original area unaffected and fail
	// {
	// 	set_last_error(EINVAL, VM_CONTRACT_OVERLAPS, EO_VM_CONTRACT);
	// 	return ERROR_OCCUR;
	// }		

	// *area = temp;				// expansion is possible so do it
	return ERROR_OK;
}

vm_area_t* vm_contract_find_area(vm_contract_t* c, uint32_t address)
{
	vm_area_t temp;
	temp.start_addr = address;
	temp.end_addr = address;

	vm_area_node_t* intersection = interval_intersect(c->root, &temp);

	if(intersection == 0)
		return 0;	
	return &intersection->area;
}

virtual_addr_t vm_contract_get_area_for_length(vm_contract_t* c, uint32_t length)
{
	// TODO: Fix this function
	// if (c == 0 || c->contract.count == 0)
	// {
	// 	set_last_error(EINVAL, VM_CONTRACT_BAD_ARGUMENTS, EO_VM_CONTRACT);
	// 	return 0;
	// }

	// for (uint32 i = 0; i < c->contract.count - 1; i++)
	// {
	// 	// test two successive area for in-between empty length
	// 	uint32 start = vm_area_get_start_address(&c->contract.data[i + 1]);
	// 	uint32 end = vm_area_get_end_address(&c->contract.data[i]) + 1;

	// 	if (start - end >= length)
	// 		return end;
	// }

	// set_last_error(EINVAL, VM_CONTRACT_NOT_FOUND, EO_VM_CONTRACT);
	// return 0;
}

void vm_contract_print(vm_contract_t* c)
{
	vm_contract_print_rec(c->root);
}