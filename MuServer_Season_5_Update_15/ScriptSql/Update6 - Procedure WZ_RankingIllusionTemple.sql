CREATE Procedure [dbo].[WZ_RankingIllusionTemple] 
@Account varchar(10),
@Name varchar(10),
@Score int
AS
BEGIN

SET NOCOUNT ON
SET XACT_ABORT ON

IF NOT EXISTS (SELECT Name FROM RankingIllusionTemple WHERE Name=@Name)
BEGIN

	 INSERT INTO RankingIllusionTemple (Name,Score) VALUES (@Name,@Score)

END
ELSE
BEGIN

	UPDATE RankingIllusionTemple SET Score=Score+@Score WHERE Name=@Name

END

SET NOCOUNT OFF
SET XACT_ABORT OFF

END
