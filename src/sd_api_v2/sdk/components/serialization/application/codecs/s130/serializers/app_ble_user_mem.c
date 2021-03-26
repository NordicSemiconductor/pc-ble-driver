#include "app_ble_user_mem.h"
#include "ser_config.h"
#include "nrf_error.h"
#include <stddef.h>

ser_ble_user_mem_t m_app_user_mem_table[SER_MAX_CONNECTIONS];

uint32_t app_ble_user_mem_context_create(uint16_t conn_handle, uint32_t *p_index)
{
  uint32_t err_code = NRF_ERROR_NO_MEM;
  uint32_t i;

  for (i=0; i<SER_MAX_CONNECTIONS; i++ )
  {
    if ( ! m_app_user_mem_table[i].conn_active )
    {
        m_app_user_mem_table[i].conn_active = 1;
        m_app_user_mem_table[i].conn_handle = conn_handle;
        *p_index = i;
        err_code = NRF_SUCCESS;
        break;
    }
  }

  return err_code;
}

uint32_t app_ble_user_mem_context_destroy(uint16_t conn_handle)
{
  uint32_t err_code = NRF_ERROR_NOT_FOUND;
  uint32_t i;

  for (i=0; i<SER_MAX_CONNECTIONS; i++ )
  {
    if (  m_app_user_mem_table[i].conn_handle == conn_handle )
    {
        m_app_user_mem_table[i].conn_active = 0;
        err_code = NRF_SUCCESS;
        break;
    }
  }

  return err_code;
}

uint32_t app_ble_user_mem_context_find(uint16_t conn_handle, uint32_t *p_index)
{
  uint32_t err_code = NRF_ERROR_NOT_FOUND;
  uint32_t i;

  for (i=0; i<SER_MAX_CONNECTIONS; i++ )
  {
    if ( (m_app_user_mem_table[i].conn_handle == conn_handle) && (m_app_user_mem_table[i].conn_active == 1) )
    {
        *p_index = i;
        err_code = NRF_SUCCESS;
        break;
    }
  }

  return err_code;
}
