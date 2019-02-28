#include <gtest/gtest.h>

<<<<<<< HEAD
class BatteryProperties : public ::testing::Test
{
protected:
	
	// general capacity
	double q;
	double SOC_min;
	double SOC_max;
	double SOC_init;

	void SetUp()
	{
		q = 100;
		SOC_init = 100;
		SOC_min = 20;
		SOC_max = 100;
	}
};
=======
#include <lib_battery.h>

#include "lib_battery_test.h"
>>>>>>> pr/11

/// Test  lithium ion battery capacity response
TEST_F(BatteryTest, LithiumIonCapacityTest)
{
	q = 100;
	SOC_init = 100;
	SOC_min = 20;
	SOC_max = 100;

<<<<<<< HEAD
	void SetUp()
	{
		BatteryProperties::SetUp();
		capacity_model = new capacity_lithium_ion_t(q, SOC_init, SOC_max, SOC_min);
=======
	if (capacityModel) {
		delete capacityModel;
>>>>>>> pr/11
	}
	capacityModel = new capacity_lithium_ion_t(q, SOC_init, SOC_max, SOC_min);

	// Check that initial capacity is equal to max
	EXPECT_EQ(capacityModel->SOC(), SOC_max);

	// Check that discharge of battery results in correct capacity
	double I = 10;
	capacityModel->updateCapacity(I, 1);
	EXPECT_EQ(capacityModel->SOC(), 90);
	EXPECT_EQ(capacityModel->q0(), 90);

	// check that charge of battery results in correct capacity
	I = -10;
	capacityModel->updateCapacity(I, 1);
	EXPECT_EQ(capacityModel->SOC(), 100);
	EXPECT_EQ(capacityModel->q0(), 100);

	// check that updating thermal behavior changes capacity as expected
	capacityModel->updateCapacityForThermal(95);
	capacityModel->check_SOC();
	EXPECT_EQ(capacityModel->q0(), 95);
	EXPECT_EQ(capacityModel->qmax(), 100);
	capacityModel->updateCapacityForThermal(100);
	capacityModel->check_SOC();

	// check that updating lifetime degradation changes capacity
	capacityModel->updateCapacityForLifetime(95);
	EXPECT_EQ(capacityModel->q0(), 95);
	EXPECT_EQ(capacityModel->qmax(), 95);

	// check that battery replacement works
	capacityModel->replace_battery();
	EXPECT_EQ(capacityModel->SOC(), SOC_max);
	EXPECT_EQ(capacityModel->q0(), 100);
	EXPECT_EQ(capacityModel->qmax(), 100);

	// check that model correctly detects overcharge, undercharge
	capacityModel->updateCapacity(I, 1);
	EXPECT_EQ(capacityModel->q0(), 100);
	EXPECT_EQ(capacityModel->SOC(), SOC_max);

	I = 110;
	capacityModel->updateCapacity(I, 1);
	EXPECT_EQ(capacityModel->q0(), 20);
	EXPECT_EQ(capacityModel->SOC(), SOC_min);
}

TEST_F(BatteryTest, LossesModel)
{
	size_t idx = 1000;

<<<<<<< HEAD
	void SetUp()
	{
		BatteryProperties::SetUp();

		q20 = 415;
		q10 = 374;
		q1 = 340;
		t1 = 5;
	}
};

class LeadAcidBattery : public LeadAcidDC4006
{
protected:
	capacity_kibam_t * capacity_model;

	void SetUp()
	{
		LeadAcidDC4006::SetUp();
		capacity_model = new capacity_kibam_t(q20, t1, q1, q10, SOC_init, SOC_max, SOC_min);
	}
	void TearDown()
	{
		if (capacity_model)
			delete capacity_model;
	}
};

TEST_F(LeadAcidBattery, LeadAcidCapacityUnitTest_lib_battery)
{
	// Check that initial capacity is equal to max
	EXPECT_EQ(capacity_model->SOC(), SOC_max);

	/*
	// Check that discharge of battery results in correct capacity
	capacity_model->updateCapacity(10, 1);
	EXPECT_DOUBLE_EQ(capacity_model->SOC(), 97.959525493854386);
	EXPECT_DOUBLE_EQ(capacity_model->q0(), 480.08208482298681);

	// check that charge of battery results in correct capacity
	capacity_model->updateCapacity(-10, 1);
	EXPECT_DOUBLE_EQ(capacity_model->SOC(), 99.872284504899923);
	EXPECT_DOUBLE_EQ(capacity_model->q0(), 489.45617406195834);

	// check that updating thermal behavior changes capacity as expected
	capacity_model->updateCapacityForThermal(95);
	EXPECT_DOUBLE_EQ(capacity_model->q0(), 465.57798058183749);
	EXPECT_DOUBLE_EQ(capacity_model->qmax(), 490.08208482298681);
	capacity_model->updateCapacityForThermal(100);

	// check that updating lifetime degradation changes capacity
	capacity_model->updateCapacityForLifetime(95);
	EXPECT_DOUBLE_EQ(capacity_model->q0(), 465.57798058183749);
	EXPECT_DOUBLE_EQ(capacity_model->qmax(), 465.57798058183749);

	// check that battery replacement works
	capacity_model->replace_battery();
	EXPECT_DOUBLE_EQ(capacity_model->SOC(), SOC_max); 

	/*
	EXPECT_DOUBLE_EQ(capacity_model->q0(), 465.57798058183749);
	EXPECT_DOUBLE_EQ(capacity_model->qmax(), 490.08208482298681);

	// check that model correctly detects overcharge, undercharge
	capacity_model->updateCapacity(-100, 1);
	EXPECT_EQ(capacity_model->q0(), 100);
	EXPECT_EQ(capacity_model->SOC(), SOC_max);
	capacity_model->updateCapacity(1000, 1);
	EXPECT_EQ(capacity_model->q0(), 20);
	EXPECT_EQ(capacity_model->SOC(), SOC_min);
	*/
	
=======
	// Return loss for february
	lossModel->run_losses(idx);
	EXPECT_EQ(lossModel->getLoss(idx), 1);
>>>>>>> pr/11

}