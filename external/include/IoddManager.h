/*!
 * @file IoddManager.h
 * @brief IoddManager class used for IODD storage and loading
 *
 * @copyright 2021, Balluff GmbH, all rights reserved
 * @author See AUTHORS file
 * @since 11.08.2022
 */
#ifndef IOL_DATAPROVIDER_IODDSTORE_H
#define IOL_DATAPROVIDER_IODDSTORE_H
#include <memory>
#include <vector>
#include <nlohmann/json.hpp>

class IoddManager
{
    public:
        IoddManager();
        ~IoddManager();

        IoddManager(IoddManager const &) = delete;
        IoddManager &operator=(IoddManager const &) = delete;

        IoddManager(IoddManager &&) noexcept;
        IoddManager &operator=(IoddManager &&) noexcept;
        
        std::tuple<nlohmann::json, nlohmann::json> interpretProcessData(std::vector<uint8_t> rawProcessData, uint16_t VendorID, uint32_t DeviceID, uint8_t RevisionID);

    private:
        class IoddManagerImpl;
        std::unique_ptr<IoddManagerImpl> mIoddManager;
};

#endif // IOL_DATAPROVIDER_IODDSTORE_H