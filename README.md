A distributed database, complete with multiversion concurrency control, deadlock detection, replication, and failure recovery.


# ReproZip

./reprozip trace scripts/test.sh inputs outputs bin/main
./reprozip pack repcrec_draft_1
./reprounzip directory setup repcrec_draft_2.rpz workspace-1/
./reprounzip directory run workspace-1/

