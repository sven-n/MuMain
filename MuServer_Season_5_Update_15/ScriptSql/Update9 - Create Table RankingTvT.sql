CREATE TABLE [dbo].[RankingTvT](
	[Name] [varchar](10) NOT NULL,
	[Kills] [int] NOT NULL,
	[Deads] [int] NOT NULL
) ON [PRIMARY]
GO

ALTER TABLE [dbo].[RankingTvT] ADD  CONSTRAINT [DF_RankingTvT_Kills]  DEFAULT ((0)) FOR [Kills]
GO

ALTER TABLE [dbo].[RankingTvT] ADD  CONSTRAINT [DF_RankingTvT_Deads]  DEFAULT ((0)) FOR [Deads]
GO


