#pragma once
#include "core/SWnd.h"
/**
 * @class      SStatic
 * @brief      ��̬�ı��ؼ���
 *
 * Describe    ��̬�ı��ؼ���֧�ֶ��У��ж�������ʱ��\n����ǿ�ƻ���
 * Usage       <text>inner text example</text>
 */
class SOUI_EXP ColorText : public SWindow
{
    SOUI_CLASS_NAME(ColorText, L"colortext")
public:
    /**
     * SStatic::SStatic
     * @brief    ���캯��
     *
     * Describe  ���캯��
     */
    ColorText();
    /**
     * SStatic::SDrawText
     * @brief    �����ı�
     * @param    IRenderTarget *pRT -- �����豸���
     * @param    LPCTSTR pszBuf -- �ı������ַ���
     * @param    int cchText -- �ַ�������
     * @param    LPRECT pRect -- ָ����νṹRECT��ָ��
     * @param    UINT uFormat --  ���ĵĻ���ѡ��
     *
     * Describe  ��DrawText��װ
     */
    virtual void DrawText(IRenderTarget* pRT, LPCTSTR pszBuf, int cchText, LPRECT pRect, UINT uFormat);

protected:
   
    SOUI_ATTRS_BEGIN()
        SOUI_ATTRS_END()
};