
CREATE TABLE [dbo].[CustomNpcQuest](
	[Name] [varchar](10) NOT NULL,
	[Quest] [int] NOT NULL,
	[Count] [int] NOT NULL,
	[MonsterCount] [int] NOT NULL,
 CONSTRAINT [PK_CustomNpcQuest] PRIMARY KEY CLUSTERED 
(
	[Name] ASC,
	[Quest] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]

GO

SET ANSI_PADDING OFF
GO

ALTER TABLE [dbo].[CustomNpcQuest]  WITH CHECK ADD  CONSTRAINT [FK_CustomNpcQuest_Character] FOREIGN KEY([Name])
REFERENCES [dbo].[Character] ([Name])
ON UPDATE CASCADE
ON DELETE CASCADE
GO

ALTER TABLE [dbo].[CustomNpcQuest] CHECK CONSTRAINT [FK_CustomNpcQuest_Character]
GO

ALTER TABLE [dbo].[CustomNpcQuest] ADD  CONSTRAINT [DF_CustomNpcQuest_Quest]  DEFAULT ((0)) FOR [Quest]
GO

ALTER TABLE [dbo].[CustomNpcQuest] ADD  CONSTRAINT [DF_CustomNpcQuest_Count]  DEFAULT ((0)) FOR [Count]
GO

ALTER TABLE [dbo].[CustomNpcQuest] ADD  CONSTRAINT [DF_CustomNpcQuest_MonsterQtd]  DEFAULT ((0)) FOR [MonsterCount]
GO


