/*
grpsort.cpp

��㯯� ���஢��

*/

/* Revision: 1.03 11.02.2001 $ */

/*
Modify:
  11.02.2001 SVS
    ! �������� DIF_VAREDIT ��������� ������ ࠧ��� ��� ��᪨
  11.02.2001 SVS
    ! ��᪮�쪮 ��筥��� ���� � �裡 � ��������ﬨ � ������� MenuItem
  13.07.2000 SVS
    ! ������� ���४樨 �� �ᯮ�짮����� new/delete/realloc
  25.06.2000 SVS
    ! �����⮢�� Master Copy
    ! �뤥����� � ����⢥ ᠬ����⥫쭮�� �����
*/

#include "headers.hpp"
#pragma hdrstop

/* $ 30.06.2000 IS
   �⠭����� ���������
*/
#include "internalheaders.hpp"
/* IS $ */

GroupSort::GroupSort()
{
  int I;
  char GroupName[80],GroupStr[GROUPSORT_MASK_SIZE], *Ptr;

  GroupData=NULL;
  GroupCount=0;
  for (I=0;;I++)
  {
    sprintf(GroupName,"UpperGroup%d",I);
    GetRegKey("SortGroups",GroupName,GroupStr,"",sizeof(GroupStr));
    if (*GroupStr==0)
      break;
    if(!(Ptr=(char *)malloc(strlen(GroupStr)+1)))
      break;
    struct GroupSortData *NewGroupData=(struct GroupSortData *)realloc(GroupData,sizeof(*GroupData)*(GroupCount+1));
    if (NewGroupData==NULL)
      break;
    GroupData=NewGroupData;
    GroupData[GroupCount].Masks=Ptr;
    strcpy(GroupData[GroupCount].Masks,GroupStr);
    GroupData[GroupCount].Group=I;
    GroupCount++;
  }
  for (I=0;;I++)
  {
    sprintf(GroupName,"LowerGroup%d",I);
    GetRegKey("SortGroups",GroupName,GroupStr,"",sizeof(GroupStr));
    if (*GroupStr==0)
      break;
    if(!(GroupData[GroupCount].Masks=(char *)malloc(strlen(GroupStr)+1)))
      break;
    struct GroupSortData *NewGroupData=(struct GroupSortData *)realloc(GroupData,sizeof(*GroupData)*(GroupCount+1));
    if (NewGroupData==NULL)
      break;
    GroupData=NewGroupData;
    strcpy(GroupData[GroupCount].Masks,GroupStr);
    GroupData[GroupCount].Group=I+DEFAULT_SORT_GROUP+1;
    GroupCount++;
  }
}


GroupSort::~GroupSort()
{
  if(GroupData)
  {
    for(int I=0; I < GroupCount; ++I)
      if(GroupData[I].Masks)
        free(GroupData[I].Masks);
    free(GroupData);
  }
}


int GroupSort::GetGroup(char *Path)
{
  for (int I=0;I<GroupCount;I++)
  {
    struct GroupSortData *CurGroupData=&GroupData[I];
    char ArgName[NM],*NamePtr=CurGroupData->Masks;
    while ((NamePtr=GetCommaWord(NamePtr,ArgName))!=NULL)
      if (CmpName(ArgName,Path))
        return(CurGroupData->Group);
  }
  return(DEFAULT_SORT_GROUP);
}


void GroupSort::EditGroups()
{
  int UpperGroup,LowerGroup,I,Pos=0,StartGroupCount=GroupCount;

  while (Pos!=-1)
    Pos=EditGroupsMenu(Pos);

  for (UpperGroup=LowerGroup=I=0;I<Max(GroupCount,StartGroupCount);I++)
  {
    char GroupName[100];
    if (I<GroupCount)
    {
      if (GroupData[I].Group<DEFAULT_SORT_GROUP)
      {
        sprintf(GroupName,"UpperGroup%d",UpperGroup);
        GroupData[I].Group=UpperGroup++;
      }
      else
      {
        sprintf(GroupName,"LowerGroup%d",LowerGroup);
        GroupData[I].Group=DEFAULT_SORT_GROUP+1+LowerGroup++;
      }
      SetRegKey("SortGroups",GroupName,GroupData[I].Masks);
    }
    else
    {
      sprintf(GroupName,"UpperGroup%d",UpperGroup++);
      DeleteRegValue("SortGroups",GroupName);
      sprintf(GroupName,"LowerGroup%d",LowerGroup++);
      DeleteRegValue("SortGroups",GroupName);
    }
  }
  CtrlObject->LeftPanel->Update(UPDATE_KEEP_SELECTION);
  CtrlObject->LeftPanel->Redraw();
  CtrlObject->RightPanel->Update(UPDATE_KEEP_SELECTION);
  CtrlObject->RightPanel->Redraw();
}


int GroupSort::EditGroupsMenu(int Pos)
{
  char NewMasks[GROUPSORT_MASK_SIZE], *Ptr;
  struct MenuItem ListItem;
  struct MenuItem ListItem2;
  memset(&ListItem,0,sizeof(ListItem));

  VMenu GroupList(MSG(MSortGroupsTitle),NULL,0,ScrY-4);
  GroupList.SetFlags(MENU_WRAPMODE|MENU_SHOWAMPERSAND);
  GroupList.SetHelp("SortGroups");
  GroupList.SetPosition(-1,-1,0,0);
  GroupList.SetBottomTitle(MSG(MSortGroupsBottom));

  int GroupPos=0,I;
  for (I=0;GroupPos<GroupCount;I++)
  {
    if (GroupData[GroupPos].Group>DEFAULT_SORT_GROUP)
      break;
    strncpy(ListItem.Name,GroupData[GroupPos].Masks,sizeof(ListItem.Name));
    ListItem.Selected=(I == Pos);
    GroupList.AddItem(&ListItem);
    GroupPos++;
  }
  int UpperGroupCount=GroupPos;

  *ListItem.Name=0;
  ListItem.Selected=(I == Pos);
  GroupList.AddItem(&ListItem);
  ListItem.Selected=0;
  ListItem.Separator=TRUE;
  GroupList.AddItem(&ListItem);
  ListItem.Separator=0;
  I+=2;

  memset(&ListItem2,0,sizeof(ListItem2));
  for (;GroupPos<GroupCount;I++)
  {
    strncpy(ListItem2.Name,GroupData[GroupPos].Masks,sizeof(ListItem2.Name));
    ListItem2.Selected=(I == Pos);
    GroupList.AddItem(&ListItem2);
    GroupPos++;
  }

  *ListItem.Name=0;
  ListItem.Selected=(GroupCount+2 == Pos);
  GroupList.AddItem(&ListItem);

  GroupList.Show();

  while (1)
  {
    while (!GroupList.Done())
    {
      int SelPos=GroupList.GetSelectPos();
      int ListPos=SelPos;
      int Key=GroupList.ReadInput();
      int UpperGroup=(SelPos<UpperGroupCount+1);
      if (ListPos>=UpperGroupCount)
      {
        if (ListPos==UpperGroupCount+1 || ListPos==UpperGroupCount &&
            Key!=KEY_INS && Key!=KEY_ESC)
        {
          GroupList.ProcessInput();
          GroupList.ClearDone();
          continue;
        }
        if (ListPos>UpperGroupCount+1)
          ListPos-=2;
        else
          ListPos=UpperGroupCount;
      }
      switch(Key)
      {
        case KEY_DEL:
          if (ListPos<GroupCount)
          {
            char GroupName[1024];
            sprintf(GroupName,"\"%s\"",GroupData[ListPos].Masks);
            if (Message(MSG_WARNING,2,MSG(MSortGroupsTitle),
                        MSG(MSortGroupsAskDel),GroupName,
                        MSG(MDelete),MSG(MCancel))!=0)
              break;

            for (int I=ListPos+1;I<GroupCount;I++)
              GroupData[I-1]=GroupData[I];
            GroupCount--;
            return(SelPos);
          }
          break;
        case KEY_INS:
          {
            if (GetString(MSG(MSortGroupsTitle),MSG(MSortGroupsEnter),"Masks","",NewMasks,sizeof(NewMasks)))
            {
              if(!(Ptr=(char *)malloc(strlen(NewMasks)+1)))
                break;
              struct GroupSortData *NewGroupData=(struct GroupSortData *)realloc(GroupData,sizeof(*GroupData)*(GroupCount+1));
              if (NewGroupData==NULL)
                break;
              GroupData=NewGroupData;
              GroupCount++;
              for (int I=GroupCount-1;I>ListPos;I--)
                GroupData[I]=GroupData[I-1];
              GroupData[ListPos].Masks=Ptr;
              strncpy(GroupData[ListPos].Masks,NewMasks,strlen(NewMasks));
              GroupData[ListPos].Group=UpperGroup ? 0:DEFAULT_SORT_GROUP+1;
              return(SelPos);
            }
          }
          break;
        case KEY_F4:
        case KEY_ENTER:
          if (ListPos<GroupCount)
          {
            strcpy(NewMasks,GroupData[ListPos].Masks);
            if (GetString(MSG(MSortGroupsTitle),MSG(MSortGroupsEnter),"Masks",NewMasks,NewMasks,sizeof(NewMasks)))
            {
              Ptr=GroupData[ListPos].Masks;
              if(strlen(Ptr) < strlen(NewMasks))
                Ptr=(char *)realloc(Ptr,strlen(NewMasks)+1);
              if(Ptr)
                break;
              GroupData[ListPos].Masks=Ptr;
              strcpy(GroupData[ListPos].Masks,NewMasks);
              GroupData[ListPos].Group=UpperGroup ? 0:DEFAULT_SORT_GROUP+1;
              return(SelPos);
            }
          }
          break;
        default:
          GroupList.ProcessInput();
          break;
      }
    }
    if (GroupList.GetExitCode()!=-1)
    {
      GroupList.ClearDone();
      GroupList.WriteInput(KEY_F4);
      continue;
    }
    break;
  }
  return(-1);
}

