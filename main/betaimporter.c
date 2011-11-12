#include <string.h>
#include <betaimporter.h>
#include <lookup.h>
#include <fixed.h>
#include <track_node.h>
#include <util.h>

static void assign_path_beta(fixed tmin, fixed tref, blind_path_result path, int path_dist) {
    fixed path_dist_fixed = fixed_new(path_dist);
    for(int i = 0; i < path.depth; i++){
        track_edge *edge = path.edges[i];
        fixed edge_dist_fixed = fixed_new(edge->dist);
        fixed edge_beta = fixed_div(fixed_mul(tmin, edge_dist_fixed), fixed_mul(path_dist_fixed, tref));
        if (strcmp(path.edges[0]->reverse->dest->reverse->name, "B16") == 0) {
//        	PRINT("%s->%s,%F", edge->reverse->dest->reverse->name, edge->dest->name, edge_beta);
        }
        //curedge->reverse->dest->reverse->name, curedge->dest->name//
        edge->beta = edge_beta;
        edge->reverse->beta = edge_beta;
    }
}

void populate_beta(lookup *track_lookup) {
	int path_dist;
	track_node *start_node, *end_node;
	fixed tmin;
	blind_path_result path;

fixed tref = fixed_new(115);

	path.depth = 0;
	start_node = lookup_get(track_lookup, "A11");
	end_node = lookup_get(track_lookup, "C7");
	ASSERT(start_node, "failed to lookup A11");
	ASSERT(strcmp(start_node->name, "A11") == 0, "Expected: A11, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup C7");
	ASSERT(strcmp(end_node->name, "C7") == 0, "Expected: C7, what i got: %s", end_node->name);
	tmin = fixed_new(125);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. A11->C7");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "A15");
	end_node = lookup_get(track_lookup, "A11");
	ASSERT(start_node, "failed to lookup A15");
	ASSERT(strcmp(start_node->name, "A15") == 0, "Expected: A15, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup A11");
	ASSERT(strcmp(end_node->name, "A11") == 0, "Expected: A11, what i got: %s", end_node->name);
	tmin = fixed_new(122);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. A15->A11");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "A4");
	end_node = lookup_get(track_lookup, "B16");
	ASSERT(start_node, "failed to lookup A4");
	ASSERT(strcmp(start_node->name, "A4") == 0, "Expected: A4, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup B16");
	ASSERT(strcmp(end_node->name, "B16") == 0, "Expected: B16, what i got: %s", end_node->name);
	tmin = fixed_new(68);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. A4->B16");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "A6");
	end_node = lookup_get(track_lookup, "B10");
	ASSERT(start_node, "failed to lookup A6");
	ASSERT(strcmp(start_node->name, "A6") == 0, "Expected: A6, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup B10");
	ASSERT(strcmp(end_node->name, "B10") == 0, "Expected: B10, what i got: %s", end_node->name);
	tmin = fixed_new(99);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. A6->B10");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "A7");
	end_node = lookup_get(track_lookup, "B12");
	ASSERT(start_node, "failed to lookup A7");
	ASSERT(strcmp(start_node->name, "A7") == 0, "Expected: A7, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup B12");
	ASSERT(strcmp(end_node->name, "B12") == 0, "Expected: B12, what i got: %s", end_node->name);
	tmin = fixed_new(74);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. A7->B12");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "A9");
	end_node = lookup_get(track_lookup, "B8");
	ASSERT(start_node, "failed to lookup A9");
	ASSERT(strcmp(start_node->name, "A9") == 0, "Expected: A9, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup B8");
	ASSERT(strcmp(end_node->name, "B8") == 0, "Expected: B8, what i got: %s", end_node->name);
	tmin = fixed_new(44);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. A9->B8");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "B1");
	end_node = lookup_get(track_lookup, "D14");
	ASSERT(start_node, "failed to lookup B1");
	ASSERT(strcmp(start_node->name, "B1") == 0, "Expected: B1, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup D14");
	ASSERT(strcmp(end_node->name, "D14") == 0, "Expected: D14, what i got: %s", end_node->name);
	tmin = fixed_new(63);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. B1->D14");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "B14");
	end_node = lookup_get(track_lookup, "D16");
	ASSERT(start_node, "failed to lookup B14");
	ASSERT(strcmp(start_node->name, "B14") == 0, "Expected: B14, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup D16");
	ASSERT(strcmp(end_node->name, "D16") == 0, "Expected: D16, what i got: %s", end_node->name);
	tmin = fixed_new(32);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. B14->D16");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "B16");
	end_node = lookup_get(track_lookup, "C10");
	ASSERT(start_node, "failed to lookup B16");
	ASSERT(strcmp(start_node->name, "B16") == 0, "Expected: B16, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup C10");
	ASSERT(strcmp(end_node->name, "C10") == 0, "Expected: C10, what i got: %s", end_node->name);
	tmin = fixed_new(56);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. B16->C10");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "B16");
	end_node = lookup_get(track_lookup, "C5");
	ASSERT(start_node, "failed to lookup B16");
	ASSERT(strcmp(start_node->name, "B16") == 0, "Expected: B16, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup C5");
	ASSERT(strcmp(end_node->name, "C5") == 0, "Expected: C5, what i got: %s", end_node->name);
	tmin = fixed_new(74);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. B16->C5");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "B6");
	end_node = lookup_get(track_lookup, "C12");
	ASSERT(start_node, "failed to lookup B6");
	ASSERT(strcmp(start_node->name, "B6") == 0, "Expected: B6, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup C12");
	ASSERT(strcmp(end_node->name, "C12") == 0, "Expected: C12, what i got: %s", end_node->name);
	tmin = fixed_new(56);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. B6->C12");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "C10");
	end_node = lookup_get(track_lookup, "B1");
	ASSERT(start_node, "failed to lookup C10");
	ASSERT(strcmp(start_node->name, "C10") == 0, "Expected: C10, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup B1");
	ASSERT(strcmp(end_node->name, "B1") == 0, "Expected: B1, what i got: %s", end_node->name);
	tmin = fixed_new(54);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. C10->B1");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "C10");
	end_node = lookup_get(track_lookup, "C2");
	ASSERT(start_node, "failed to lookup C10");
	ASSERT(strcmp(start_node->name, "C10") == 0, "Expected: C10, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup C2");
	ASSERT(strcmp(end_node->name, "C2") == 0, "Expected: C2, what i got: %s", end_node->name);
	tmin = fixed_new(86);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. C10->C2");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "C12");
	end_node = lookup_get(track_lookup, "A4");
	ASSERT(start_node, "failed to lookup C12");
	ASSERT(strcmp(start_node->name, "C12") == 0, "Expected: C12, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup A4");
	ASSERT(strcmp(end_node->name, "A4") == 0, "Expected: A4, what i got: %s", end_node->name);
	tmin = fixed_new(53);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. C12->A4");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "C14");
	end_node = lookup_get(track_lookup, "A14");
	ASSERT(start_node, "failed to lookup C14");
	ASSERT(strcmp(start_node->name, "C14") == 0, "Expected: C14, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup A14");
	ASSERT(strcmp(end_node->name, "A14") == 0, "Expected: A14, what i got: %s", end_node->name);
	tmin = fixed_new(96);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. C14->A14");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "C14");
	end_node = lookup_get(track_lookup, "A15");
	ASSERT(start_node, "failed to lookup C14");
	ASSERT(strcmp(start_node->name, "C14") == 0, "Expected: C14, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup A15");
	ASSERT(strcmp(end_node->name, "A15") == 0, "Expected: A15, what i got: %s", end_node->name);
	tmin = fixed_new(108);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. C14->A15");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "C14");
	end_node = lookup_get(track_lookup, "A2");
	ASSERT(start_node, "failed to lookup C14");
	ASSERT(strcmp(start_node->name, "C14") == 0, "Expected: C14, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup A2");
	ASSERT(strcmp(end_node->name, "A2") == 0, "Expected: A2, what i got: %s", end_node->name);
	tmin = fixed_new(72);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. C14->A2");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "C14");
	end_node = lookup_get(track_lookup, "A4");
	ASSERT(start_node, "failed to lookup C14");
	ASSERT(strcmp(start_node->name, "C14") == 0, "Expected: C14, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup A4");
	ASSERT(strcmp(end_node->name, "A4") == 0, "Expected: A4, what i got: %s", end_node->name);
	tmin = fixed_new(90);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. C14->A4");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "C15");
	end_node = lookup_get(track_lookup, "D12");
	ASSERT(start_node, "failed to lookup C15");
	ASSERT(strcmp(start_node->name, "C15") == 0, "Expected: C15, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup D12");
	ASSERT(strcmp(end_node->name, "D12") == 0, "Expected: D12, what i got: %s", end_node->name);
	tmin = fixed_new(62);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. C15->D12");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "C2");
	end_node = lookup_get(track_lookup, "E2");
	ASSERT(start_node, "failed to lookup C2");
	ASSERT(strcmp(start_node->name, "C2") == 0, "Expected: C2, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup E2");
	ASSERT(strcmp(end_node->name, "E2") == 0, "Expected: E2, what i got: %s", end_node->name);
	tmin = fixed_new(75);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. C2->E2");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "C5");
	end_node = lookup_get(track_lookup, "C15");
	ASSERT(start_node, "failed to lookup C5");
	ASSERT(strcmp(start_node->name, "C5") == 0, "Expected: C5, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup C15");
	ASSERT(strcmp(end_node->name, "C15") == 0, "Expected: C15, what i got: %s", end_node->name);
	tmin = fixed_new(43);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. C5->C15");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "C5");
	end_node = lookup_get(track_lookup, "C3");
	ASSERT(start_node, "failed to lookup C5");
	ASSERT(strcmp(start_node->name, "C5") == 0, "Expected: C5, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup C3");
	ASSERT(strcmp(end_node->name, "C3") == 0, "Expected: C3, what i got: %s", end_node->name);
	tmin = fixed_new(129);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. C5->C3");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "C7");
	end_node = lookup_get(track_lookup, "C3");
	ASSERT(start_node, "failed to lookup C7");
	ASSERT(strcmp(start_node->name, "C7") == 0, "Expected: C7, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup C3");
	ASSERT(strcmp(end_node->name, "C3") == 0, "Expected: C3, what i got: %s", end_node->name);
	tmin = fixed_new(92);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. C7->C3");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "C7");
	end_node = lookup_get(track_lookup, "E11");
	ASSERT(start_node, "failed to lookup C7");
	ASSERT(strcmp(start_node->name, "C7") == 0, "Expected: C7, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup E11");
	ASSERT(strcmp(end_node->name, "E11") == 0, "Expected: E11, what i got: %s", end_node->name);
	tmin = fixed_new(125);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. C7->E11");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "C8");
	end_node = lookup_get(track_lookup, "A6");
	ASSERT(start_node, "failed to lookup C8");
	ASSERT(strcmp(start_node->name, "C8") == 0, "Expected: C8, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup A6");
	ASSERT(strcmp(end_node->name, "A6") == 0, "Expected: A6, what i got: %s", end_node->name);
	tmin = fixed_new(53);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. C8->A6");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "C8");
	end_node = lookup_get(track_lookup, "A7");
	ASSERT(start_node, "failed to lookup C8");
	ASSERT(strcmp(start_node->name, "C8") == 0, "Expected: C8, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup A7");
	ASSERT(strcmp(end_node->name, "A7") == 0, "Expected: A7, what i got: %s", end_node->name);
	tmin = fixed_new(84);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. C8->A7");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "C8");
	end_node = lookup_get(track_lookup, "A9");
	ASSERT(start_node, "failed to lookup C8");
	ASSERT(strcmp(start_node->name, "C8") == 0, "Expected: C8, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup A9");
	ASSERT(strcmp(end_node->name, "A9") == 0, "Expected: A9, what i got: %s", end_node->name);
	tmin = fixed_new(114);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. C8->A9");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "D1");
	end_node = lookup_get(track_lookup, "B14");
	ASSERT(start_node, "failed to lookup D1");
	ASSERT(strcmp(start_node->name, "D1") == 0, "Expected: D1, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup B14");
	ASSERT(strcmp(end_node->name, "B14") == 0, "Expected: B14, what i got: %s", end_node->name);
	tmin = fixed_new(71);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. D1->B14");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "D1");
	end_node = lookup_get(track_lookup, "C1");
	ASSERT(start_node, "failed to lookup D1");
	ASSERT(strcmp(start_node->name, "D1") == 0, "Expected: D1, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup C1");
	ASSERT(strcmp(end_node->name, "C1") == 0, "Expected: C1, what i got: %s", end_node->name);
	tmin = fixed_new(73);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. D1->C1");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "D10");
	end_node = lookup_get(track_lookup, "D5");
	ASSERT(start_node, "failed to lookup D10");
	ASSERT(strcmp(start_node->name, "D10") == 0, "Expected: D10, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup D5");
	ASSERT(strcmp(end_node->name, "D5") == 0, "Expected: D5, what i got: %s", end_node->name);
	tmin = fixed_new(110);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. D10->D5");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "D10");
	end_node = lookup_get(track_lookup, "D8");
	ASSERT(start_node, "failed to lookup D10");
	ASSERT(strcmp(start_node->name, "D10") == 0, "Expected: D10, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup D8");
	ASSERT(strcmp(end_node->name, "D8") == 0, "Expected: D8, what i got: %s", end_node->name);
	tmin = fixed_new(122);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. D10->D8");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "D12");
	end_node = lookup_get(track_lookup, "E11");
	ASSERT(start_node, "failed to lookup D12");
	ASSERT(strcmp(start_node->name, "D12") == 0, "Expected: D12, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup E11");
	ASSERT(strcmp(end_node->name, "E11") == 0, "Expected: E11, what i got: %s", end_node->name);
	tmin = fixed_new(44);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. D12->E11");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "D14");
	end_node = lookup_get(track_lookup, "E14");
	ASSERT(start_node, "failed to lookup D14");
	ASSERT(strcmp(start_node->name, "D14") == 0, "Expected: D14, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup E14");
	ASSERT(strcmp(end_node->name, "E14") == 0, "Expected: E14, what i got: %s", end_node->name);
	tmin = fixed_new(44);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. D14->E14");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "D16");
	end_node = lookup_get(track_lookup, "E14");
	ASSERT(start_node, "failed to lookup D16");
	ASSERT(strcmp(start_node->name, "D16") == 0, "Expected: D16, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup E14");
	ASSERT(strcmp(end_node->name, "E14") == 0, "Expected: E14, what i got: %s", end_node->name);
	tmin = fixed_new(44);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. D16->E14");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "D4");
	end_node = lookup_get(track_lookup, "B6");
	ASSERT(start_node, "failed to lookup D4");
	ASSERT(strcmp(start_node->name, "D4") == 0, "Expected: D4, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup B6");
	ASSERT(strcmp(end_node->name, "B6") == 0, "Expected: B6, what i got: %s", end_node->name);
	tmin = fixed_new(59);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. D4->B6");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "D5");
	end_node = lookup_get(track_lookup, "E6");
	ASSERT(start_node, "failed to lookup D5");
	ASSERT(strcmp(start_node->name, "D5") == 0, "Expected: D5, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup E6");
	ASSERT(strcmp(end_node->name, "E6") == 0, "Expected: E6, what i got: %s", end_node->name);
	tmin = fixed_new(44);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. D5->E6");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "D8");
	end_node = lookup_get(track_lookup, "E8");
	ASSERT(start_node, "failed to lookup D8");
	ASSERT(strcmp(start_node->name, "D8") == 0, "Expected: D8, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup E8");
	ASSERT(strcmp(end_node->name, "E8") == 0, "Expected: E8, what i got: %s", end_node->name);
	tmin = fixed_new(56);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. D8->E8");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "E1");
	end_node = lookup_get(track_lookup, "B14");
	ASSERT(start_node, "failed to lookup E1");
	ASSERT(strcmp(start_node->name, "E1") == 0, "Expected: E1, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup B14");
	ASSERT(strcmp(end_node->name, "B14") == 0, "Expected: B14, what i got: %s", end_node->name);
	tmin = fixed_new(70);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. E1->B14");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "E11");
	end_node = lookup_get(track_lookup, "D10");
	ASSERT(start_node, "failed to lookup E11");
	ASSERT(strcmp(start_node->name, "E11") == 0, "Expected: E11, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup D10");
	ASSERT(strcmp(end_node->name, "D10") == 0, "Expected: D10, what i got: %s", end_node->name);
	tmin = fixed_new(42);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. E11->D10");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "E14");
	end_node = lookup_get(track_lookup, "E9");
	ASSERT(start_node, "failed to lookup E14");
	ASSERT(strcmp(start_node->name, "E14") == 0, "Expected: E14, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup E9");
	ASSERT(strcmp(end_node->name, "E9") == 0, "Expected: E9, what i got: %s", end_node->name);
	tmin = fixed_new(43);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. E14->E9");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "E15");
	end_node = lookup_get(track_lookup, "C12");
	ASSERT(start_node, "failed to lookup E15");
	ASSERT(strcmp(start_node->name, "E15") == 0, "Expected: E15, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup C12");
	ASSERT(strcmp(end_node->name, "C12") == 0, "Expected: C12, what i got: %s", end_node->name);
	tmin = fixed_new(53);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. E15->C12");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "E2");
	end_node = lookup_get(track_lookup, "E15");
	ASSERT(start_node, "failed to lookup E2");
	ASSERT(strcmp(start_node->name, "E2") == 0, "Expected: E2, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup E15");
	ASSERT(strcmp(end_node->name, "E15") == 0, "Expected: E15, what i got: %s", end_node->name);
	tmin = fixed_new(30);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. E2->E15");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "E3");
	end_node = lookup_get(track_lookup, "D1");
	ASSERT(start_node, "failed to lookup E3");
	ASSERT(strcmp(start_node->name, "E3") == 0, "Expected: E3, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup D1");
	ASSERT(strcmp(end_node->name, "D1") == 0, "Expected: D1, what i got: %s", end_node->name);
	tmin = fixed_new(29);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. E3->D1");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "E6");
	end_node = lookup_get(track_lookup, "D4");
	ASSERT(start_node, "failed to lookup E6");
	ASSERT(strcmp(start_node->name, "E6") == 0, "Expected: E6, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup D4");
	ASSERT(strcmp(end_node->name, "D4") == 0, "Expected: D4, what i got: %s", end_node->name);
	tmin = fixed_new(42);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. E6->D4");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "E6");
	end_node = lookup_get(track_lookup, "E3");
	ASSERT(start_node, "failed to lookup E6");
	ASSERT(strcmp(start_node->name, "E6") == 0, "Expected: E6, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup E3");
	ASSERT(strcmp(end_node->name, "E3") == 0, "Expected: E3, what i got: %s", end_node->name);
	tmin = fixed_new(43);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. E6->E3");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "E8");
	end_node = lookup_get(track_lookup, "C14");
	ASSERT(start_node, "failed to lookup E8");
	ASSERT(strcmp(start_node->name, "E8") == 0, "Expected: E8, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup C14");
	ASSERT(strcmp(end_node->name, "C14") == 0, "Expected: C14, what i got: %s", end_node->name);
	tmin = fixed_new(115);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. E8->C14");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "E9");
	end_node = lookup_get(track_lookup, "D5");
	ASSERT(start_node, "failed to lookup E9");
	ASSERT(strcmp(start_node->name, "E9") == 0, "Expected: E9, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup D5");
	ASSERT(strcmp(end_node->name, "D5") == 0, "Expected: D5, what i got: %s", end_node->name);
	tmin = fixed_new(97);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. E9->D5");
    assign_path_beta(tmin, tref, path, path_dist);


	path.depth = 0;
	start_node = lookup_get(track_lookup, "E9");
	end_node = lookup_get(track_lookup, "D8");
	ASSERT(start_node, "failed to lookup E9");
	ASSERT(strcmp(start_node->name, "E9") == 0, "Expected: E9, what i got: %s", start_node->name);
	ASSERT(end_node, "failed to lookup D8");
	ASSERT(strcmp(end_node->name, "D8") == 0, "Expected: D8, what i got: %s", end_node->name);
	tmin = fixed_new(109);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, "find_path_blind failed. E9->D8");
    assign_path_beta(tmin, tref, path, path_dist);
//    while(1);
}

