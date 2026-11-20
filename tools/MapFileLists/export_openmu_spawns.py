"""Write the monsters and NPCs each map spawns, from an initialized OpenMU database, as the input of the per-map file
lists (tests/monsters/test_map_file_lists.cpp).

    python tools/MapFileLists/export_openmu_spawns.py [--database openmu] [--psql <path>]

Connects with psql and the usual PGHOST, PGPORT, PGUSER and PGPASSWORD variables. Every spawn area counts, event waves
included, so the list says what the server can spawn on the map, not what stands there at start. The database must hold
one game configuration: the spawns of several would mix.
"""
import argparse
import json
import pathlib
import shutil
import subprocess

OUTPUT = pathlib.Path(__file__).with_name("openmu-spawns.json")
QUERY = """
select map."Number", monster."Number"
from config."MonsterSpawnArea" spawn
join config."GameMapDefinition" map on map."Id" = spawn."GameMapId"
join config."MonsterDefinition" monster on monster."Id" = spawn."MonsterDefinitionId"
group by 1, 2
"""
CONFIGURATIONS = 'select count(distinct "GameConfigurationId") from config."GameMapDefinition"'


def group(rows):
    """{"<map>": [monster, ...]} from "map,monster" lines, sorted by number."""
    maps = {}
    for line in rows:
        if line.strip():
            map_number, monster = (int(value) for value in line.split(","))
            maps.setdefault(map_number, set()).add(monster)
    return {str(number): sorted(maps[number]) for number in sorted(maps)}


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--database", default="openmu")
    parser.add_argument("--psql", default=shutil.which("psql") or "psql")
    arguments = parser.parse_args()

    def query(sql):
        return subprocess.run([arguments.psql, "-d", arguments.database, "-A", "-t", "-F", ",", "-c", sql],
                              capture_output=True, text=True, check=True).stdout

    configurations = int(query(CONFIGURATIONS))
    if configurations != 1:
        raise SystemExit(f"the database holds {configurations} game configurations, not one")
    spawns = group(query(QUERY).splitlines())
    lines = ",\n".join(f'  "{number}": {json.dumps(monsters)}' for number, monsters in spawns.items())
    OUTPUT.write_text('{\n "source": "OpenMU",\n "maps": {\n' + lines + "\n }\n}\n", encoding="utf-8", newline="\n")
    print(f"{OUTPUT}: {len(spawns)} maps")


assert group(["2,5", "0,7", "0,3", "0,7", ""]) == {"0": [3, 7], "2": [5]}

if __name__ == "__main__":
    main()
