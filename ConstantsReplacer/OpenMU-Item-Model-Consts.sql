SELECT 'MODEL_' || upper(
	replace(replace(replace(replace("Name", ' ', '_'), '(', ''), ')', ''),'''', '')
	) || ' = MODEL_' ||
	(
		CASE
			WHEN "Group" = 0 THEN 'SWORD'
		    WHEN "Group" = 1 THEN 'AXE'
		    WHEN "Group" = 2 THEN 'MACE'
		    WHEN "Group" = 3 THEN 'SPEAR'
		    WHEN "Group" = 4 THEN 'BOW'
		    WHEN "Group" = 5 THEN 'STAFF'
		    WHEN "Group" = 6 THEN 'SHIELD'
		    WHEN "Group" = 7 THEN 'HELM'
		    WHEN "Group" = 8 THEN 'ARMOR'
		    WHEN "Group" = 9 THEN 'PANTS'
		    WHEN "Group" = 10 THEN 'GLOVES'
		    WHEN "Group" = 11 THEN 'BOOTS'
			WHEN "Group" = 12 THEN 'WING'
			WHEN "Group" = 13 THEN 'HELPER'
			WHEN "Group" = 14 THEN 'POTION'
			WHEN "Group" = 15 THEN 'ETC'
		END
	)
	||
	' + ' || "Number" ||','
	FROM config."ItemDefinition"
	ORDER BY "Group", "Number"
