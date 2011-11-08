#!/bin/bash

outfile=main/betaimporter.c
betafile=main/beta.csv

echo '
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
        edge->beta = edge_beta;
        edge->reverse->beta = edge_beta;
    }
}


void populate_beta(lookup *track_lookup) {
	int path_dist;
	track_node *start_node, *end_node;
	fixed tmin;
	blind_path_result path;
' > $outfile

tref=$(head -1 $betafile | cut -d',' -f1)

echo "fixed tref = fixed_new($tref);" >> $outfile


tail +2 $betafile |
while read line; do
	startnode=$(echo $line | cut -d',' -f1)
	endnode=$(echo $line | cut -d',' -f2)
	tmin=$(echo $line | cut -d',' -f3)

	echo "
	path.depth = 0;
	start_node = lookup_get(track_lookup, \"$startnode\");
	end_node = lookup_get(track_lookup, \"$endnode\");
	ASSERT(start_node, \"failed to lookup $startnode\");
	ASSERT(strcmp(start_node->name, \"$startnode\") == 0, \"Expected: $startnode, what i got: %s\", start_node->name);
	ASSERT(end_node, \"failed to lookup $endnode\");
	ASSERT(strcmp(end_node->name, \"$endnode\") == 0, \"Expected: $endnode, what i got: %s\", end_node->name);
	tmin = fixed_new($tmin);
	path_dist = find_path_blind(start_node, end_node, &path, 1);
	ASSERT(path_dist >= 0, \"find_dist_blind failed. $startnode->$endnode\");
    assign_path_beta(tmin, tref, path, path_dist);
    " >> $outfile
done

echo "}
">> $outfile
