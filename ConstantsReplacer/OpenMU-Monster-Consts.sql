SELECT ('MONSTER_' || up."NAME_UPPER" || ' = ' || "Number" || ',') monster_consts,
	   ('MODEL_' || up."NAME_UPPER" || ' = MODEL_MONSTER01 + ' || "Number" || ',') monster_model_consts
FROM (
	SELECT "Number", 
	(upper(
		replace(replace(replace(replace(replace("Designation", ' ', '_'), '(', ''), ')', ''),'''', ''),'-','')
		)) AS "NAME_UPPER"
		FROM config."MonsterDefinition"
		) as up
	ORDER BY "Number"